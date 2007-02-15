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


/* static variables */
ModuleContainer manager::detectionModules;
DetectModExporter* manager::exporter = 0;
bool manager::restartOnCrash = false;
bool manager::shutdown = false;


manager::manager(DetectModExporter* exporter)
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

#ifdef IDMEF_SUPPORT_ENABLED
        // TODO: remove this after Raimondas merged his topas-dynamic-control into trunk/topas
        topasID = "this_is_a_dummy_id_please_remove_me";
#endif
	mutex.lock();
}


manager::~manager()
{
#ifdef IDMEF_SUPPORT_ENABLED
        msg(MSG_DEBUG, "Disconnecting from xmlBlaster servers");
        for (unsigned i = 0; i != commObjs.size(); ++i) {
                commObjs[i]->disconnect();
                delete commObjs[i];
        }
#endif
}

void manager::addDetectionModule(const std::string& modulePath, const std::vector<std::string>& arguments) 
{
        if (modulePath.size() == 0) {
                msg(MSG_ERROR, "Manager: Got empty path to detection module");
        }
        
        /* cancle action when file doesn't exist */
        struct stat buffer;
        if (-1 == lstat(modulePath.c_str(), &buffer)) {
                msg(MSG_ERROR, "Can't stat %s: %s", modulePath.c_str(), strerror(errno));
                return;
        }

	detectionModules.createModule(modulePath, arguments);
}

void manager::startModules()
{
#ifdef IDMEF_SUPPORT_ENABLED
        if (topasID.empty()) {
                throw std::runtime_error("TOPAS id is empty. Cannot start modules!");
        }
        detectionModules.topasID = config_space::TOPAS + "-" + topasID;

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
        IdmefMessage* currentMessage = new IdmefMessage(config_space::TOPAS, topasID, "", IdmefMessage::HEARTBEAT);
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

        detectionModules.startModules(exporter);
}


void manager::sigAlarm(int) 
{
        /* do nothing if collector announced system shutdown */
        if (shutdown)
                return;

        msg(MSG_ERROR, "Manager: One or more detection modules seem to parse their files to slowly.");
        /* TODO: implement some error handling */
}

void manager::sigChild(int sig) 
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
                        msg(MSG_ERROR, "Manager: Detection module with pid %i"
			    "terminated with exist state 0. Not restarting module", pid);
			detectionModules.setState(pid, DetectMod::NotRunning);
			return;
                }else {
                        msg(MSG_ERROR, "Manager: Detection module with pid %i "
			    "terminated abnormally. Return value was: %i", pid, status);
			detectionModules.setState(pid, DetectMod::Crashed);
                }
        } else {
                if (WIFSIGNALED(status)) {
                        msg(MSG_ERROR, "Manager: Detection module with pid %i was "
			    "terminated by signal %i", pid, WTERMSIG(status));
			detectionModules.setState(pid, DetectMod::Crashed);
                }
        }
        
        if (restartOnCrash) {
                msg(MSG_INFO, "Manager: Restarting crashed module");
                detectionModules.restartCrashedModule(pid, exporter);
        } else {
                msg(MSG_INFO, "Manager: Not restarting crashed module");
                detectionModules.deleteModule(pid);
        }
}

void* manager::run(void* data) 
{
        manager* man = (manager*)data;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        
	while (!shutdown) {
		man->lockMutex();
		
		alarm(man->killTime);
		detectionModules.notifyAll(exporter);
		exporter->clearSink();
		alarm(0);
 
#ifdef IDMEF_SUPPORT_ENABLED
                for (unsigned i = 0; i != man->commObjs.size(); ++i) {
			std::string ret = man->commObjs[i]->getUpdateMessage();
                        if (ret != "") {
                                XMLConfObj* confObj = new XMLConfObj(ret, XMLConfObj::XML_STRING);
                                if (NULL != confObj) {
                                        man->update(confObj);
                                }
                        }
                }
#endif               
	}
        return NULL;
}

void manager::newPacket() 
{
        unlockMutex();
}

void manager::lockMutex() 
{
	mutex.lock();
}

void manager::unlockMutex() 
{
	mutex.unlock();
}

void manager::prepareShutdown()
{
        shutdown = true;
}

void manager::killModules()
{
	detectionModules.killDetectionModules();
}

#ifdef IDMEF_SUPPORT_ENABLED
void manager::update(XMLConfObj* xmlObj)
{
	std::cout << "Update for topas received!" << std::endl;
        if (xmlObj->nodeExists("start")) {
		std::cout << "-> starting module..." << std::endl;
        } else if (xmlObj->nodeExists("stop")) {
		std::cout << "-> stoping module..." << std::endl;
        } else { // add your commands here
		std::cout << "-> unknown operation" << std::endl;
        }
        delete xmlObj;
}
#endif
