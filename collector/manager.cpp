/**************************************************************************/
/*    Copyright (C) 2005-2007 Lothar Braun <mail@lobraun.de>              */
/*                                                                        */
/*    This library is free software; you can redistribute it and/or       */
/*    modify it under the terms of the GNU Lesser General Public          */
/*    License as published by the Free Software Foundation; either        */
/*    version 2.1 of the License, or (at your option) any later version.  */
/*                                                                        */
/*    This library is distributed in the hope that it will be useful,     */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/*    Lesser General Public License for more details.                     */
/*                                                                        */
/*    You should have received a copy of the GNU Lesser General Public    */
/*    License along with this library; if not, write to the Free Software  */
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA    */
/**************************************************************************/

#include "manager.h"
#include "detectmodexporter.h"


#include <commonutils/exceptions.h>
#include <concentrator/msg.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include <fstream>

/* static variables */
ModuleContainer Manager::runningModules;
DetectModExporter* Manager::exporter = 0;
bool Manager::restartOnCrash = false;
bool Manager::shutdown = false;


Manager::Manager(DetectModExporter* exporter)
        : killTime(config_space::DEFAULT_KILL_TIME)
{       
        this->exporter = exporter;
        /* install signal handlers */
        if (SIG_ERR == signal(SIGALRM, sigAlarm)) {
                msg(MSG_ERROR, "Manager: Couldn't install signal handler for SIGALRM.\n"
		               "Failure in detection modules won't be detected");
        }
        if (SIG_ERR == signal(SIGCHLD, sigChild)) {
                msg(MSG_ERROR, "Manager: Couldn't install signal handler for SIGCHLD.\n"
		    "Crashing detection modules won't be detected");
        }

	mutex.lock();
}


Manager::~Manager()
{
#ifdef IDMEF_SUPPORT_ENABLED
        msg(MSG_DEBUG, "Disconnecting from xmlBlaster servers");
        for (unsigned i = 0; i != commObjs.size(); ++i) {
		std::string managerID = (*xmlBlasters[i].getElement()).getProperty().getProperty(config_space::MANAGER_ID);
		if (managerID == "") {
			msg(MSG_INFO, ("Using default " + config_space::MANAGER_ID + " \""
				       + config_space::DEFAULT_MANAGER_ID + "\"").c_str());
			managerID = config_space::DEFAULT_MANAGER_ID;
		}
		commObjs[i]->erase(config_space::TOPAS + "-" + topasID);
		commObjs[i]->publish("<exit oid='" + config_space::TOPAS + "-" + topasID + "'/>", managerID);
                commObjs[i]->disconnect();
                delete commObjs[i];
        }
#endif
}

void Manager::addDetectionModule(const std::string& modulePath,
				 const std::string& configFile,
				 std::vector<std::string>& arguments,
				 ModuleState s)
{
        if (modulePath.size() == 0) {
                msg(MSG_ERROR, "Manager: Got empty path to detection module");
        }
        
        /* cancel action when file doesn't exist */
        struct stat buffer;
        if (-1 == lstat(modulePath.c_str(), &buffer)) {
                msg(MSG_ERROR, "Can't stat %s: %s", modulePath.c_str(), strerror(errno));
                return;
        }

	arguments.insert(arguments.begin(), 1, configFile);
	availableModules[modulePath] = arguments;

	if (s == start) {
		runningModules.createModule(modulePath, arguments);
	}
}

void Manager::startModules()
{
#ifdef IDMEF_SUPPORT_ENABLED
        if (topasID.empty()) {
                throw std::runtime_error("TOPAS id is empty. Cannot start modules!");
        }
        runningModules.topasID = config_space::TOPAS + "-" + topasID;

        /* connect to all xmlBlaster servers */
        msg(MSG_INFO, "Connecting to xmlBlaster servers");
        for (unsigned i = 0; i != xmlBlasters.size(); ++i) {
                try {
                        XmlBlasterCommObject* comm = new XmlBlasterCommObject(*xmlBlasters[i].getElement());
                        comm->connect();
                        commObjs.push_back(comm);
                } catch (const XmlBlasterException &e) {
                        msg(MSG_FATAL, "Cannot connect to xmlBlaster: ", e.what());
                        throw std::runtime_error("Make sure, the xmlBlaster server is up and running.");
                }
        }
        /* send <Heartbeat> message to all xmlBlaster servers and subscribe for update messages */
        IdmefMessage* currentMessage = new IdmefMessage(config_space::TOPAS, "", "", IdmefMessage::HEARTBEAT);
	currentMessage->setAnalyzerAttr("", config_space::TOPAS, "", "");
	currentMessage->createAnalyzerNode("ipv4-addr", "127.0.0.1", "255.255.255.255", "B305");
	currentMessage->setAnalyzerNodeIdAttr(config_space::TOPAS + "-" + topasID);
        for (unsigned i = 0; i != commObjs.size(); ++i) {
		std::string managerID = (*xmlBlasters[i].getElement()).getProperty().getProperty(config_space::MANAGER_ID);
                if (managerID == "") {
                        msg(MSG_INFO, ("Using default " + config_space::MANAGER_ID + " \""
                                       + config_space::DEFAULT_MANAGER_ID + "\"").c_str());
                        managerID = config_space::DEFAULT_MANAGER_ID;
                }
                currentMessage->publish(*commObjs[i], managerID);
                commObjs[i]->subscribe(config_space::TOPAS + "-" + topasID, XmlBlasterCommObject::MESSAGE);
        }
        delete currentMessage;
#endif

        runningModules.startModules(exporter);
}


void Manager::sigAlarm(int) 
{
        /* do nothing if collector announced system shutdown */
        if (shutdown)
                return;

        msg(MSG_ERROR, "Manager: One or more detection modules seem to parse their files to slowly.");
	runningModules.findAndKillSlowModule();
}

void Manager::sigChild(int sig) 
{
        if (shutdown)
                return;

        msg(MSG_ERROR, "Manager: A detection module exited.");
        int status;
        pid_t pid = wait(&status);
        if (pid == -1)
                msg(MSG_ERROR, "Manager: Can't determine exit state from "
		    "detecton module: %s", strerror(errno));

        if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                        msg(MSG_ERROR, "Manager: Detection module with pid %i "
			    "terminated with exit state 0. Not restarting module", pid);
			runningModules.setState(pid, DetectMod::Remove);
			return;
                }else {
                        msg(MSG_ERROR, "Manager: Detection module with pid %i "
			    "terminated abnormally. Return value was: %i", pid, status);
			runningModules.setState(pid, DetectMod::Crashed);
                }
        } else {
                if (WIFSIGNALED(status)) {
                        msg(MSG_ERROR, "Manager: Detection module with pid %i was "
			    "terminated by signal %i", pid, WTERMSIG(status));
			runningModules.setState(pid, DetectMod::Crashed);
                }
        }
        
        if (restartOnCrash) {
                msg(MSG_INFO, "Manager: Restarting crashed module");
                runningModules.restartCrashedModule(pid, exporter);
        } else {
                msg(MSG_INFO, "Manager: Not restarting crashed module");
                runningModules.deleteModule(pid);
        }
}

void* Manager::run(void* data) 
{
        Manager* man = (Manager*)data;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        
	while (!shutdown) {
		man->lockMutex();
		
		alarm(man->killTime);
		runningModules.notifyAll(exporter);
		exporter->clearSink();
		alarm(0);
 
#ifdef IDMEF_SUPPORT_ENABLED
                for (unsigned i = 0; i != man->commObjs.size(); ++i) {
			std::string ret = man->commObjs[i]->getUpdateMessage();
                        if (ret != "") {
				try {
					XMLConfObj* confObj = new XMLConfObj(ret, XMLConfObj::XML_STRING);
					man->update(confObj);
					delete confObj;
				} catch (const exceptions::XMLException &e) {
					msg(MSG_ERROR, e.what());
					man->sendControlMessage("<result oid='" + config_space::TOPAS + "-" + man->topasID + "'>Manager: " + 
								std::string(e.what()) + "</result>");
				}
			}
                }
#endif               
	}
        return NULL;
}

void Manager::newPacket() 
{
        unlockMutex();
}

void Manager::lockMutex() 
{
	mutex.lock();
}

void Manager::unlockMutex() 
{
	mutex.unlock();
}

void Manager::prepareShutdown()
{
        shutdown = true;
}

void Manager::killModules()
{
	runningModules.killDetectionModules();
}

#ifdef IDMEF_SUPPORT_ENABLED
void Manager::update(XMLConfObj* xmlObj)
{
	msg(MSG_INFO, "Update for topas received!");

	/* start a module */
        if (xmlObj->nodeExists(config_space::START)) {
		try {
			std::string filename = xmlObj->getAttribute(config_space::START, config_space::MODULE_FILENAME);
			std::string config_file = xmlObj->getAttribute(config_space::START, config_space::CONFIG_FILE);
			if (availableModules.find(filename) != availableModules.end()) {
				msg(MSG_INFO, "Manager: starting module...");
				availableModules[filename][0] = config_file;
				runningModules.createModule(filename, availableModules[filename]);
				runningModules.startModules(exporter);
				sendControlMessage("<result oid=\"" + config_space::TOPAS + "-" + topasID + "\">Manager: module \"" + 
						   filename + "\" started</result>");
			} else {
				msg(MSG_ERROR, ("Manager: module \"" + filename + "\" is not available").c_str());
				sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "\">Manager: module '" + 
						   filename + "\" is not available</result>");
			}
		} catch (const exceptions::XMLException &e) {
			msg(MSG_ERROR, e.what());
			sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>Manager: " + std::string(e.what()) + 
					   "</result>");
		}
	/* get module configuration file */
	} else if (xmlObj->nodeExists(config_space::GET_MODULE_CONFIG)) {
		try {
			std::string filename = xmlObj->getAttribute(config_space::GET_MODULE_CONFIG, config_space::MODULE_FILENAME);
			if (availableModules.find(filename) != availableModules.end()) {
				msg(MSG_INFO, "Manager: retrieving module configuration file...");
				std::string config_file = availableModules[filename][0];
				std::ifstream inputStream;
				char* line = NULL;
				inputStream.open(config_file.c_str(), std::ios::in);
				if (!inputStream) {
					msg(MSG_ERROR, ("Manager: Can't open \"" + config_file + "\"").c_str());
					sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + 
							   "'>Manager: Can't open \"" + config_file + "\"</result>");
				} else {
					std::string message;
					std::string line;
					while (std::getline(inputStream, line)) {
						message += line + "\n";
					}
					sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>\n" + message + 
							   "\n</result>");
					inputStream.close();
				}
			} else {
				msg(MSG_ERROR, ("Manager: module \"" + filename + "\" is not available").c_str());
				sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>Manager: module \"" + 
						   filename + "\" is not available</result>");
			}
		} catch (const exceptions::XMLException &e) {
			msg(MSG_ERROR, e.what());
			sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>Manager: " + std::string(e.what()) + 
					   "</result>");
		}
	/* update module configuration file */
	} else if (xmlObj->nodeExists(config_space::UPDATE_MODULE_CONFIG)) {
		try {
			std::string filename = xmlObj->getAttribute(config_space::UPDATE_MODULE_CONFIG, config_space::MODULE_FILENAME);
			if (availableModules.find(filename) != availableModules.end()) {
				msg(MSG_INFO, "Manager: updating module configuration file...");
				std::string config_file = availableModules[filename][0];
				std::ofstream outputStream;
				outputStream.open(config_file.c_str(), std::ios::trunc);
				if (!outputStream) {
					msg(MSG_ERROR, ("Manager: Can't open \"" + config_file + "\"").c_str());
					sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>Manager: Can't open \"" + 
							   config_file + "\" for writing</result>");
				} else {
					xmlObj->enterNode(config_space::UPDATE_MODULE_CONFIG);
					xmlObj->enterNode(config_space::CONFIGURATION);
					std::string message = xmlObj->toString();
					outputStream.write(message.c_str(), message.size());
					sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>Update succesful</result>");
					outputStream.close();
				}
			} else {
				msg(MSG_ERROR, ("Manager: module \"" + filename + "\" is not available").c_str());
				sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>Manager: module \"" + filename + 
						   "\" is not available</result>");
			}
		} catch (const exceptions::XMLException &e) {
			msg(MSG_ERROR, e.what());
			sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>Manager: " + std::string(e.what()) + 
					   "</result>");
		}
        }
	/* get available modules */
	else if (xmlObj->nodeExists(config_space::GET_AVAILABLE_MODULES)) {
		std::map<std::string,std::vector<std::string> >::const_iterator iter; 
		std::string message = "";
		for (iter = availableModules.begin(); iter != availableModules.end(); iter++) {
			message += "<module " + config_space::MODULE_FILENAME + "=\"" + iter->first + "\" " 
				+ config_space::CONFIG_FILE + "=\"" + iter->second[0] + "\"/>\n";
		}
		if (message != "") {
			sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>\n" + message + "</result>");
		} else {
			sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>No modules available</result>");
		}
        }
	/* get running modules */
	else if (xmlObj->nodeExists(config_space::GET_RUNNING_MODULES)) {
		try {
			std::string message = "";
			std::vector<std::string> modules = runningModules.getRunningModules();
			for (unsigned i = 0; i != modules.size(); ++i) {
				unsigned seperatorPos;
				if (std::string::npos != (seperatorPos = modules[i].find(' '))) {
					std::string fileName = std::string(modules[i].begin(), modules[i].begin() + seperatorPos);
					std::string configFile  = std::string(modules[i].begin() + seperatorPos + 1, modules[i].end());
					message += "<module " + config_space::MODULE_FILENAME + "=\"" + fileName + "\" " 
						+ config_space::CONFIG_FILE + "=\"" + configFile + "\"/>\n";
				}
			}
			if (message != "") {
				sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>\n" + message + "</result>");
			} else {
				sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + 
						   "'>No running modules available</result>");
			}
		} catch (const exceptions::XMLException &e) {
			msg(MSG_ERROR, e.what());
			sendControlMessage("<result oid='" + config_space::TOPAS + "-" + topasID + "'>Manager: " + std::string(e.what()) + 
					   "</result>");
		}
        }
	/* add commands here */
	else { 
		msg(MSG_INFO, "Manager: unknown operation");
        }
}

void Manager::sendControlMessage(const std::string& message)
{
	for (unsigned i = 0; i != commObjs.size(); ++i) {
		std::string managerID = (*xmlBlasters[i].getElement()).getProperty().getProperty(config_space::MANAGER_ID);
		if (managerID == "") {
			managerID = config_space::DEFAULT_MANAGER_ID;
		}
		commObjs[i]->publish(message, managerID);
	}
}

#endif
