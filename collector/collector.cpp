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

Manager* Collector::man = 0;
bool Collector::terminateCollector = false;
DetectModExporter* Collector::exporter = NULL;
RecorderBase* Collector::recorder = NULL;
bool Collector::replaying = false;
Metering* Collector::metering = 0;

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

Collector::Collector() 
{
        exporter = new DetectModExporter();
        man = new Manager(exporter);
        listenPort = config_space::DEFAULT_LISTEN_PORT;
        receiverType = config_space::DEFAULT_TRANSPORT_PROTO;
	recorder = new RecorderOff();
}


Collector::~Collector() 
{
	msg(MSG_DEBUG, "Entering Collector::~Collector()");
	msg(MSG_DEBUG, "Deleting manager");
	delete man; man = 0;
	msg(MSG_DEBUG, "Deleting exporter");
	delete exporter; exporter = 0;
	msg(MSG_DEBUG, "Deleting recorder");
	delete recorder; recorder = 0;
	msg(MSG_DEBUG, "Cleaning packet directory");
	::cleanPacketDir(packetDir);
	msg(MSG_DEBUG, "Leaving Collector::~Collector()");
}

void Collector::readConfig(const std::string& configFile) 
{
        CollectorConfObj* config = new CollectorConfObj(configFile);

	try {
		/* get and set working directory */
		config->enterNode(config_space::COLLECTOR_STRING);

		if (config->nodeExists(config_space::WORKING_DIR)) {
			readWorkingDir(config);
		} else {
			msg(MSG_INFO, "No working directory specified. "
			    "Assuming current directory");
		}

		if (config->nodeExists(config_space::DETECTIONMODULES)) {
			readDetectionModules(config);
		} else {
			msg(MSG_INFO, "No detection modules to start.");
		}
		
		readMisc(config);

		if (config->nodeExists(config_space::EXCHANGE_PROTOCOL)) {
			readExchangeProtocol(config);
		} else {
			throw exceptions::ConfigError("No exchange protocol defined!");
		}

		if (config->nodeExists(config_space::PLAYER)) {
			readRecording(config);
		}

#ifdef IDMEF_SUPPORT_ENABLED
		if (config->nodeExists(config_space::XMLBLASTERS)) {
			readIDMEF(config);
		} else {
			throw exceptions::ConfigError("No <" + config_space::XMLBLASTERS
						      + "> statement in config file");
		}
#endif
		config->leaveNode();

		delete config;
		
		Metering::setDirectoryName("metering/");
	} catch(exceptions::XMLException& e) {
		msg(MSG_FATAL, "Error configuring collector: %s", e.what());
		delete config;
		throw e;
	}
}

void Collector::readWorkingDir(XMLConfObj* config)
{
	std::string tmp;
	tmp = config->getValue(config_space::WORKING_DIR);
	if (-1 == chdir(tmp.c_str())) {
		msg(MSG_FATAL, "Failed to set working directory to %s: %s",
		    tmp.c_str(), strerror(errno));
		throw exceptions::ConfigError(std::string("Failed to set "
							  "working directory to ") +
					      tmp.c_str() + ": " 
					      + strerror(errno));
	} else {
		msg(MSG_INFO, "Successfully changed working direcory "
		    "to: %s", tmp.c_str());
	}
}

void Collector::readDetectionModules(XMLConfObj* config)
{
	std::string tmp;	
	/* get the names of the detection modules */
	config->enterNode(config_space::DETECTIONMODULES);
	if (config->nodeExists(config_space::DETECTIONMODULE)) {
		config->setNode(config_space::DETECTIONMODULE);
		while (config->nextNodeExists()) {
			config->enterNextNode();
			std::string filename = config->getValue(config_space::FILENAME);
			std::string configFile = config->getValue(config_space::CONFIG_FILE);
			std::string run = config->getValue(config_space::RUN);
			std::vector<std::string> args;
			if (config->nodeExists(config_space::ARG)) {
				args.push_back(config->getValue(config_space::ARG));
				while(config->nextNodeExists()) {
					args.push_back(config->getNextValue());
				}
			}
			if (run == "yes") {
				man->addDetectionModule(filename, configFile, args, Manager::start);
			} else if (run == "no") {
				man->addDetectionModule(filename, configFile, args, Manager::dontStart);
			} else {
				throw exceptions::ConfigError("Bad value for <" + config_space::RUN
							      + ">. Expecting \"yes\" or \"no\"");
			}
			config->leaveNode();
		}
	}
	config->leaveNode();
	msg(MSG_INFO, "Extracted all detection modules from config file.");
}

void Collector::readMisc(XMLConfObj* config)
{
	std::string tmp;
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
		msg(MSG_INFO, "No timeout specified. Taking default time span: %i seconds",
		    man->killTime);
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

}

void Collector::readExchangeProtocol(XMLConfObj* config)
{
	std::string tmp;
	/* configure the exchange protocol */
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
}

void Collector::readRecording(XMLConfObj* config)
{
	std::string tmp;
	/* turn recording on/off. turned off by default */
	config->enterNode(config_space::PLAYER);
	std::string type = config->getValue(config_space::ACTION);
	if (type != config_space::OFF) {
		tmp = config->getValue(config_space::TRAFFIC_DIR);
		if (type == config_space::RECORD) {
			if (recorder)
				delete recorder;
			recorder = new FileRecorder(tmp, FileRecorder::PrepareRecording);
			replaying = false;
			msg(MSG_INFO, "Turned on recorder. IPFIX packets will be stored in %s",
			    tmp.c_str());
		} else if (type == config_space::REPLAY) {
			if (recorder)
				delete recorder;
			recorder = new FileRecorder(tmp, FileRecorder::PrepareReplaying);
			recorder->setPacketCallback(Collector::messageCallBackFunction);
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
	config->leaveNode();
}

void Collector::readIDMEF(XMLConfObj* config)
{
#ifdef IDMEF_SUPPORT_ENABLED
	std::string tmp;
	/* configure xmlBlaster connection properties */
	config->enterNode(config_space::XMLBLASTERS);
	if (!config->nodeExists(config_space::XMLBLASTER)) {
		throw exceptions::ConfigError("No <" + config_space::XMLBLASTER
					      + "> statement in config file");
	}
	config->setNode(config_space::XMLBLASTER);
	unsigned int count = 0;
	while (config->nextNodeExists()) {
		config->enterNextNode();
		/* Property does handle properties in the java-way */
		Property::MapType propMap;
		std::vector<std::string> props;
		/* get all properties */
		if (config->nodeExists(config_space::XMLBLASTER_PROP)) {
			props.push_back(config->getValue(config_space::XMLBLASTER_PROP));
			while (config->nextNodeExists()) {
				props.push_back(config->getNextValue());
			}
		} else {
			msg(MSG_INFO, ("No <" + config_space::XMLBLASTER_PROP + 
				       "> statement in config file, using default values").c_str());
		}
		for (unsigned i = 0; i != props.size(); ++i) {
			unsigned seperatorPos;
			if (std::string::npos != (seperatorPos = props[i].find(' '))) {
				std::string key = std::string(props[i].begin(), props[i].begin() + seperatorPos);
				std::string value  = std::string(props[i].begin() + seperatorPos + 1, props[i].end());
				propMap[key] = value;
			}
		}
		/* global configuration for each xmlBlaster connection */
		std::string instanceName = "connection-" + ++count;
		GlobalRef globalRef =  Global::getInstance().createInstance(instanceName, &propMap);
		/* get topas id here */
		std::string str = globalRef.getElement()->getInstanceId();
		man->topasID = std::string(str.begin() + str.find_last_of("/") + 1, str.end());
		man->xmlBlasters.push_back(globalRef);
		config->leaveNode();
	}
	config->leaveNode();
#endif
}

void Collector::startModules() 
{
	metering = new Metering("collector.stat");
        man->startModules();
}

void Collector::run()
{
	/* start manager thread */
	pthread_t managerThreadId;
	int ret_val;
	if ((ret_val = pthread_create(&managerThreadId, NULL, Manager::run, man)) != 0) {
		msg(MSG_FATAL, "Collector: Couldn't create manager thread: %s", strerror(ret_val));
		throw std::runtime_error("Collector isn't able to run without a mangager");
	}

	if (SIG_ERR == signal(SIGINT, sigInt)) {
		msg(MSG_ERROR, "Collector: Can't install signal handler for SIGINT: %s",
		    strerror(errno));
		return;
	}
	if (SIG_ERR == signal(SIGTERM, sigInt)) {
		msg(MSG_ERROR, "Collector: Can't install signal handler for SIGTERM: %s",
		    strerror(errno));
		return;
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
		packetProcessor->processPacketCallbackFunction = Collector::messageCallBackFunction;
		
		addIpfixPacketProcessor(ipfixCollector, packetProcessor);
		msg(MSG_INFO, "Starting IpfixCollector");
		startIpfixCollector(ipfixCollector);

		while (!terminateCollector) {
			pause();
		}
		msg(MSG_INFO, "Shutdown arrived, waiting 2 seconds before exit...");
		sleep(2);
		
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

int Collector::messageCallBackFunction(IpfixParser* ipfixParser, byte* data, uint16_t len) 
{
	metering->addValue();
        static int ret;
	recorder->record(data, len);
        ret = exporter->exportToSink(ipfixParser, data, len);
        man->newPacket();
        return ret;
}

void Collector::sigInt(int /*sig*/) 
{
        man->prepareShutdown();
	recorder->abort();
        terminateCollector = true;
}


void Collector::setListenPort(int port) 
{
        listenPort = port;
}

void Collector::setReceiverType(Receiver_Type r_t) 
{
        receiverType = r_t;
}
