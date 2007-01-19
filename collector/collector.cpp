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

/**
 * @author Lothar Braun <braunl@informatik.uni-tuebingen.de>
 */

#include "collector.h"
#include "manager.h"
#include "collectorconfobj.h"
#include "recorder.h"
#include "detectmodexporter.h"


#include <commonutils/global.h>
#include <commonutils/exceptions.h>
#include <commonutils/packetstats.h>


#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>


#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <iostream>


/******* Static variables ***************************/

manager* collector::man = 0;
bool collector::terminateCollector = false;
DetectModExporter* collector::exporter = NULL;
RecorderBase* collector::recorder = NULL;
bool collector::replaying = false;
Metering* collector::metering = 0;

/****** Implementation ******************************/

void cleanPacketDir(const std::string& dirName)
{

	if (dirName.empty())
		return;
	DIR* dir = opendir(dirName.c_str());
	if (NULL == dir) {
		msg(MSG_ERROR, "Could not open directory %s", dirName.c_str());
		return;
	}
	struct dirent *dent = NULL;
	struct stat buf;
	std::string filename;
	while (NULL != (dent = readdir(dir))) {
		filename = dirName + dent->d_name;
		if (-1 == lstat(filename.c_str(), &buf)) {
			msg(MSG_ERROR, "lstat error on %s: %s", filename.c_str(), strerror(errno));
			continue;
		}
		if (S_ISREG(buf.st_mode)) {
			unlink(filename.c_str());
		}
	}
	closedir(dir);
}

collector::collector() 
        : config(NULL)
{
        exporter = new DetectModExporter();
        man = new manager(exporter);
        listenPort = config_space::DEFAULT_LISTEN_PORT;
        receiverType = config_space::DEFAULT_TRANSPORT_PROTO;
	recorder = new RecorderOff();
}


collector::~collector() 
{
	msg(MSG_DEBUG, "Entering collector::~collector()");
	msg(MSG_DEBUG, "Deleting manager");
	delete man; man = 0;
	msg(MSG_DEBUG, "Deleting XMLConfObj");
	delete config; config = 0;
	msg(MSG_DEBUG, "Deleting exporter");
	delete exporter; exporter = 0;
	msg(MSG_DEBUG, "Deleting recorder");
	delete recorder; recorder = 0;
	msg(MSG_DEBUG, "Cleaning packet directory");
	::cleanPacketDir(packetDir);
	msg(MSG_DEBUG, "Leaving collector::~collector()");
}

void collector::readConfig(const std::string& configFile) 
{
        config = new CollectorConfObj(configFile);
	std::string tmp;

	try {
		/* get and set working directory */
		config->enterNode(config_space::COLLECTOR_STRING);
		if (config->nodeExists(config_space::WORKING_DIR)) {
			tmp = config->getValue(config_space::WORKING_DIR);
			if (-1 == chdir(tmp.c_str())) {
				msg(MSG_FATAL, "Failed to set working directory to %s: %s", tmp.c_str(), strerror(errno));
				throw exceptions::ConfigError(std::string("Failed to set working directory to ") +
							      tmp.c_str() + ": " + strerror(errno));
			} else {
				msg(MSG_INFO, "Successfully changed working direcory to: %s", tmp.c_str());
			}
		} else {
			msg(MSG_INFO, "No working directory specified");
		}

		/* get the names of the detection modules */
		if (config->nodeExists(config_space::DETECTIONMODULES)) {
			config->enterNode(config_space::DETECTIONMODULES);
			if (config->nodeExists(config_space::DETECTIONMODULE)) {
				config->setNode(config_space::DETECTIONMODULE);
				while (config->nextNodeExists()) {
					config->enterNextNode();
					std::string filename = config->getValue(config_space::FILENAME);
					std::vector<std::string> args;
					if (config->nodeExists(config_space::ARG)) {
						args.push_back(config->getValue(config_space::ARG));
						while(config->nextNodeExists()) {
							args.push_back(config->getNextValue());
						}
					}
					man->addDetectionModule(filename, args);
					config->leaveNode();
				}
			}
			config->leaveNode();
			msg(MSG_INFO, "Extracted all detection modules from config file.");
		} else {
			msg(MSG_INFO, "No detection modules to start.");
		}

		/* port to listen on */
		if (config->nodeExists(config_space::LISTEN_PORT)) {
			tmp = config->getValue(config_space::LISTEN_PORT);
			std::stringstream sstream(tmp);
			sstream >> listenPort;
			msg(MSG_INFO, "Listening on port %i", listenPort);
		} else {
			listenPort = config_space::DEFAULT_LISTEN_PORT;
			msg(MSG_INFO, "No port specified, taking default port %i", listenPort);
		}

		/* killtime */
		if (config->nodeExists(config_space::KILL_TIME)) {
			tmp = config->getValue(config_space::KILL_TIME);
			std::stringstream sstream(tmp);
			sstream >> man->killTime;
			msg(MSG_INFO, "Detection module kill time: %i seconds", man->killTime);
		} else {
			man->killTime = config_space::DEFAULT_KILL_TIME;
			msg(MSG_INFO, "No timeout specified. Taking default time span: %i seconds", man->killTime);
		}

		/* restart crashed modules */
		if (config->nodeExists(config_space::RESTART_ON_CRASH)) {
			if (config->nodeExists(config_space::RESTART_ON_CRASH)) {
				tmp = config->getValue(config_space::RESTART_ON_CRASH);
				if (tmp == "yes") {
					man->restartOnCrash = true;
				} else if (tmp == "no") {
					man->restartOnCrash = false;
				} else {
					throw exceptions::ConfigError("Bad value for configuration item \"" +
								      config_space::RESTART_ON_CRASH +
								      "\n Posibilities are yes or no");
				}
			} else {
				
			}
		} else {
			man->restartOnCrash = false;
		}
		
		if (man->restartOnCrash) {
			msg(MSG_INFO, "Restarting detection modules turned on");
		} else {
			msg(MSG_INFO, "Restarting detection modules turned off");
		}
		
		/* configure the exchange protocol */
		if (config->nodeExists(config_space::EXCHANGE_PROTOCOL)) {
			std::string type = config->getAttribute(config_space::EXCHANGE_PROTOCOL,
								config_space::EP_TYPE);
			if (type == config_space::EP_FILES) {
				config->enterNode(config_space::EXCHANGE_PROTOCOL);
				exporter->setExportingStyle(DetectModExporter::USE_FILES);
				/* packetdir for storing IPFIX-Files */
				if (config->nodeExists(config_space::PACKET_DIRECTORY)) {
					packetDir = config->getValue(config_space::PACKET_DIRECTORY);
					::cleanPacketDir(packetDir);
					exporter->setPacketDir(packetDir);
				} else {
					throw exceptions::ConfigError("No tmp directory for IPFIX-files specified");
				}
				config->leaveNode();
			} else if (type == config_space::EP_SHM) {
				config->enterNode(config_space::EXCHANGE_PROTOCOL);
				/* get shared memory size */
				exporter->setExportingStyle(DetectModExporter::USE_SHARED_MEMORY);
				if (config->nodeExists(config_space::SHMSIZE)) {
					unsigned shmSize;
					shmSize = atoi(config->getValue(config_space::SHMSIZE).c_str());
					exporter->setSharedMemorySize(shmSize);
				} else {
					throw exceptions::ConfigError("No shm size for IPFIX-storage specified");
				}
				config->leaveNode();
			}
		} else {
			throw exceptions::ConfigError("No exchange protocol defined!");
		}

		/* turn recording on/off. turned off by default */
		if (config->nodeExists(config_space::PLAYER)) {
			config->enterNode(config_space::PLAYER);
			std::string type = config->getValue(config_space::ACTION);
			if (type != config_space::OFF) {
				tmp = config->getValue(config_space::TRAFFIC_DIR);
				if (type == config_space::RECORD) {
					if (recorder)
						delete recorder;
					recorder = new FileRecorder(tmp, FileRecorder::PrepareRecording);
					replaying = false;
					msg(MSG_INFO, "Turned on recorder. IPFIX packets will be stored in %s", tmp.c_str());
				} else if (type == config_space::REPLAY) {
					if (recorder)
						delete recorder;
					recorder = new FileRecorder(tmp, FileRecorder::PrepareReplaying);
					recorder->setPacketCallback(collector::messageCallBackFunction);
					replaying = true;
					msg(MSG_INFO, "Collector now starts in replay mode");
				} else {
					throw exceptions::ConfigError("Only \"" + config_space::REPLAY + "\", \""
								      + config_space::RECORD + "\" or \"" 
								      + config_space::OFF + "\" are allowed for "
								      + config_space::ACTION);
				}
			} else {
				replaying = false;
			}
		}

		config->leaveNode();
		
		Metering::setDirectoryName("metering/");
	} catch(exceptions::XMLException& e) {
		msg(MSG_FATAL, "Error configuring collector: %s", e.what());
		throw e;
	}
}

void collector::startModules() 
{
	metering = new Metering("collector.stat");
        man->startModules();
}

void collector::run()
{
	/* start manager thread */
	pthread_t managerThreadId;
	int ret_val;
	if ((ret_val = pthread_create(&managerThreadId, NULL, manager::run, man)) != 0) {
		msg(MSG_FATAL, "Collector: Couldn't create manager thread: %s", strerror(ret_val));
		throw std::runtime_error("Collector isn't able to run without a mangager");
	}

	/* If the signal handler couldn't be installed, the collector will be shut down after the first signal
                   arrived. If the signal handler could be installed successful the collector will only be shut down,
                   if SIGINT arrives
	*/
	bool inst_sig_handler = true;
	if (SIG_ERR == signal(SIGINT, sigInt)) {
		msg(MSG_ERROR, "Collector: Can't install signal handler for SIGINT: %s", strerror(errno));
		inst_sig_handler = false;
	}
	if (SIG_ERR == signal(SIGTERM, sigInt)) {
		msg(MSG_ERROR, "Collector: Can't install signal handler for SIGTERM: %s", strerror(errno));
		inst_sig_handler = false;
	}

	if (replaying) {
		msg(MSG_INFO, "Starting replaying process");
		recorder->play();
		msg(MSG_INFO, "Replayed traffic. System will be shut down");
	} else {
		/* start the collecting process */
		
		msg(MSG_INFO, "Initializing IpfixCollector");
		initializeIpfixCollectors();
		IpfixCollector* ipfixCollector = createIpfixCollector();

		IpfixReceiver* ipfixReceiver = createIpfixReceiver(receiverType, listenPort);
		addIpfixReceiver(ipfixCollector, ipfixReceiver);

		msg(MSG_INFO, "Initializing PacketProcessor");
		IpfixPacketProcessor* packetProcessor = createIpfixPacketProcessor();
		packetProcessor->processPacketCallbackFunction = collector::messageCallBackFunction;
		
		addIpfixPacketProcessor(ipfixCollector, packetProcessor);
		msg(MSG_INFO, "Starting IpfixCollector");
		startIpfixCollector(ipfixCollector);
		
		if (inst_sig_handler) {
			while (!terminateCollector) {
				pause();
			}
		}
		else {
			pause();
		}
		
		msg(MSG_INFO, "Shutting down IpfixCollector");
		
		stopIpfixCollector(ipfixCollector);
		deinitializeIpfixCollectors();

		/*
		msg(MSG_INFO, "Waiting for manager to shut down");
		int err;
		if (0 != (err = pthread_join(managerThreadId, NULL))) {
			msg(MSG_ERROR, "Error waiting on manager: %s", strerror(err));
		}
		*/
		
		msg(MSG_INFO, "IpfixCollector was shut down");
	}
	msg(MSG_INFO, "Shutting down modules ...");
	man->prepareShutdown();
	man->killModules();
	msg(MSG_INFO, "Modules where shut down");
	msg(MSG_INFO, "Shutting down manager thread");
	if (ESRCH == pthread_cancel(managerThreadId)) {
		msg(MSG_ERROR, "Could not shout down manager thread: No such thread");
	}
	msg(MSG_INFO, "Manager was successfully shut down");
}

int collector::messageCallBackFunction(IpfixParser* ipfixParser, byte* data, uint16_t len) 
{
	metering->addValue();
        static int ret;
	recorder->record(data, len);
        ret = exporter->exportToSink(ipfixParser, data, len);
        man->newPacket();
        return ret;
}

void collector::sigInt(int /*sig*/) 
{
        man->prepareShutdown();
	recorder->abort();
        terminateCollector = true;
}


void collector::setListenPort(int port) 
{
        listenPort = port;
}

void collector::setReceiverType(Receiver_Type r_t) 
{
        receiverType = r_t;
}
