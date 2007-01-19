/**************************************************************************/
/*   Copyright (C) 2006-2007 Nico Weber                                   */
/*                                                                        */
/*   This library is free software; you can redistribute it and/or        */
/*   modify it under the terms of the GNU Lesser General Public           */
/*   License as published by the Free Software Foundation; either         */
/*   version 2.1 of the License, or (at your option) any later version.   */
/*                                                                        */
/*   This library is distributed in the hope that it will be useful,      */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*   Lesser General Public License for more details.                      */
/*                                                                        */
/*   You should have received a copy of the GNU Lesser General Public     */
/*   License along with this library; if not, write to the Free Software  */
/*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,           */
/*   MA  02110-1301, USA                                                  */
/**************************************************************************/

#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>

#include <sys/wait.h>
#include <errno.h>      
#include <pthread.h>
#include "snortmodule.h"

#include <concentrator/msg.h>


using namespace ConfigStrings;
bool Snortmodule::shutdown=false;

pid_t Snortmodule::pid=0;
FILE* Snortmodule::fifofd=NULL;
const char* Snortmodule::FIFO=NULL;
pcapwriter* Snortmodule::writer = new pcapwriter;
#ifdef IDMEF_SUPPORT_ENABLED
Snortmodule::wrapperConfig_t Snortmodule::wrapperConfig;
#endif

Snortmodule::Snortmodule(const std::string& filename) : DetectionBase<SnortStore, UnbufferedFilesInputPolicy<SemShmNotifier, SnortStore> >(filename)
{
	subscribeTypeId(IPFIX_TYPEID_destinationIPv4Address);
	subscribeTypeId(IPFIX_TYPEID_sourceIPv4Address);
	subscribeTypeId(IPFIX_TYPEID_sourceTransportPort);
	subscribeTypeId(IPFIX_TYPEID_destinationTransportPort);
	subscribeTypeId(IPFIX_TYPEID_tcpControlBits);
	subscribeTypeId(IPFIX_TYPEID_classOfServiceIPv4);
	subscribeTypeId(IPFIX_TYPEID_protocolIdentifier);
	subscribeTypeId(IPFIX_TYPEID_identificationV4);
	subscribeTypeId(IPFIX_TYPEID_udpSourcePort);
	subscribeTypeId(IPFIX_TYPEID_udpDestinationPort);
	subscribeTypeId(IPFIX_TYPEID_tcpSourcePort);
	subscribeTypeId(IPFIX_TYPEID_tcpDestinationPort);
	subscribeTypeId(IPFIX_TYPEID_tcpSequenceNumber);
	subscribeTypeId(IPFIX_TYPEID_tcpAcknowledgementNumber);
	subscribeTypeId(IPFIX_TYPEID_tcpWindowSize);
	subscribeTypeId(187);// | tcpUrgentPointer
	subscribeTypeId(IPFIX_TYPEID_totalLengthIPv4);
	subscribeTypeId(IPFIX_TYPEID_ipTimeToLive);
	 
	
	subscribeTypeId(313);//PSAMP_TYPEID_ipHeaderPacketSection 
	subscribeTypeId(314);//PSAMP_TYPEID_ipPayloadPacketSection  
	
	// Sighandlers		
	if (signal(SIGTERM, sigTerm) == SIG_ERR) {
               msg(MSG_ERROR, "Snortmodule: Couldn't install signal handler for SIGTERM.\n ");
        } 
	if (signal(SIGINT, sigInt) == SIG_ERR) {
               msg(MSG_ERROR, "Snortmodule: Couldn't install signal handler for SIGINT.\n ");
        } 	

	if (signal(SIGCHLD, sigChild) == SIG_ERR) {
                msg(MSG_ERROR, "Snortmodule: Couldn't install signal handler for SIGCHLD.\n Crashing of external detectionengine won't be detected");
        }
	setAlarmTime(0);	
}

Snortmodule::~Snortmodule(){
}


void Snortmodule::init(){
	msg(MSG_INFO, "Snortmodule: Setting up...");
	/* create FIFO */
	if ((mknod(FIFO, S_IFIFO | 0666, 0)) < 0){
		msg(MSG_ERROR, "Snortmodule: mknod failed");
		throw exceptions::DetectionModuleError("Snortmodule", "Can't create FIFO", strerror(errno));
	}

#ifdef IDMEF_SUPPORT_ENABLED

	if (wrapperConfig.enable){
		wrapperConfig.module = (void *) this;
		wrapperConfig_t* args=&wrapperConfig;
		if(pthread_create(&wrapperConfig.Id, NULL, Snortmodule::xmlWrapperEntry, (void *)args)){
			msg(MSG_ERROR, "Snortmodule: xmlWrapper startup FAILED");
			throw exceptions::DetectionModuleError("Snortmodule", "Wrapper failed", strerror(errno));
		}
	} 
	else msg(MSG_INFO, "Snortmodule: xmlWrapper is DISABLED");

#else 
	msg(MSG_INFO, "Snortmodule: xmlWrapper is DISABLED (not supported)");

#endif
	
	/* fork the external module */
	if (-1 == (pid = fork())) {
                throw exceptions::DetectionModuleError("Snortmodule", "Can't fork a new process for starting the module", strerror(errno));
        }

	/* child (snort) */
		if (pid == 0) {
		

	// Ugly "howto execute string from configfile" but it works
			std::vector<std::string> args;
			std::string command;
			unsigned last = 0;
			unsigned res, commandEnd = 0;
			bool more = true;
			do {
				res = execute.find(' ', last);
				if (res == std::string::npos) {
				more = false;
				res = execute.size();
			}
			if (commandEnd == 0) {
				commandEnd = res;
				command = std::string(execute.begin(), execute.begin() + res);
			} else {
				std::string arg = std::string(execute.begin() + last, execute.begin() + res);
				if (!arg.empty()) {args.push_back(arg);	}
			}
			last = res + 1; // one past last space
			} while (more);
			
						
			const char** args2 = (const char**)malloc((args.size() + 2) * sizeof(char*));
			std::string arg0(execute);
			arg0.erase(execute.find(' ',0));
			*args2 = arg0.c_str();
			for (unsigned i = 0; i != args.size(); ++i) {*(args2 + i + 1) = args[i].c_str();}
			*(args2 + args.size() + 1) = 0;
			execv(arg0.c_str(), (char* const*) args2);
			
			throw exceptions::DetectionModuleError("Snortmodule", "Can't execute the detection module", strerror(errno));
		}
  	
	/* Wait for Child to init */
	sleep(3);
	fifofd=fopen(FIFO, "w");
	if(fifofd < 0){
		msg(MSG_ERROR, "Snortmodule: Can't open FIFO");
		throw exceptions::DetectionModuleError("Snortmodule", "Can't open FIFO", strerror(errno));
	}
 	
	/*Write initial pcap file header to external application*/
	msg(MSG_INFO, "Snortmodule: External application running... Init writer and sending pcap file header");
	writer->init(fifofd,calc_thcs,calc_iphcs);
	writer->writedummypacket(); // for debug proposes only
	msg(MSG_INFO, "Snortmodule: All set up");
}


void Snortmodule::CleanExit(){
	msg(MSG_INFO, "Snortmodule: Shutting down...");
	std::cout << "Snortmodule: <---- "<<writer->get_packets_read() <<" packets read " << writer->get_packets_written() << " written and " << (writer->get_packets_read()-writer->get_packets_written()) << " dropped. ---->"<<std::endl;
	msg(MSG_INFO, "Snortmodule: Cleaning up...");
	delete writer;
	kill(pid, SIGINT);
	if ((pid=wait(NULL))!=0) msg(MSG_ERROR,"Snortmodule: Child exited with exitcode != 0 (Most of the time ok)");
	fclose(fifofd);
	unlink(FIFO);
#ifdef IDMEF_SUPPORT_ENABLED
	unlink(wrapperConfig.fifoname.c_str());
#endif
	msg(MSG_INFO, "Snortmodule: Exiting");
	//pthread_exit(NULL);
	exit(0);
}


void Snortmodule::readConfig(const std::string& filename)
{
        bool doRead = false;
        msg(MSG_INFO, "Snortmodule: Reading configuration file");

        if (!filename.empty()) {
                config = new ConfObj(filename);
                config->setSection(SNORTSECTION);
                doRead = true;
        }
        char* tmp;
	if (doRead && NULL != (tmp = config->getValue(DEBUG_LEVEL))) {
		                msg_setlevel(atoi(tmp));
	}

	if (doRead && NULL != (tmp = config->getValue(EXECUTE))) {
		execute = tmp;
	} else {
		execute = DEFAULT_EXECUTE;
	}
	if (doRead && NULL != (tmp = config->getValue(FIFOc))) {
		fifo  = tmp;
	} else {
	fifo = DEFAULT_FIFO;
	}
	FIFO=fifo.c_str();

        if (doRead && NULL != (tmp = config->getValue(CALC_THCS))) {
		                if ((std::string)tmp == "false") calc_thcs=false;
				else calc_thcs = true;
	}			        
	if (doRead && NULL != (tmp = config->getValue(CALC_IPHCS))) {
			                if ((std::string)tmp== "false") calc_iphcs= false; 
					 else calc_iphcs= true;
	}
		

        if (doRead && NULL != (tmp = config->getValue(ACCEPT_SOURCE_IDS))) {
		unsigned last = 0;
		unsigned res, IDEnd = 0;
		bool more = true;
		std::string temp,config;
		config=tmp;
		do {
			res = config.find(',', last);
			if (res == std::string::npos) {
			more = false;
			res = config.size();
		}
		if (IDEnd == 0) {
			IDEnd = res;
			temp = std::string(config.begin(), config.begin() + res);
			accept_source_ids.push_back(atoi(temp.c_str()));
		} else {
			temp = std::string(config.begin() + last, config.begin() + res);
			if (!temp.empty()) {accept_source_ids.push_back(atoi(temp.c_str())); }
		}
		last = res + 1; // one past last space
		} while (more);
		SnortStore::accept_source_id=&accept_source_ids;
	}
#ifdef IDMEF_SUPPORT_ENABLED
	config->setSection(WRAPPERSECTION);
	
	if (doRead && NULL != (tmp = config->getValue(ENABLE))) {
			if ((std::string)tmp == "false" ) wrapperConfig.enable = false; 
					 else wrapperConfig.enable= true;
	} else wrapperConfig.enable = DEFAULTENABLE;

	if (doRead && NULL != (tmp = config->getValue(WRAPPERPIPE))) {
			                wrapperConfig.fifoname = tmp; 
	} else wrapperConfig.fifoname = DEFAULTWRAPPERPIPE;
	if (doRead && NULL != (tmp = config->getValue(TOPIC))) {
			                wrapperConfig.topic = tmp; 
	} else wrapperConfig.topic = DEFAULTTOPIC;
#endif
}


void Snortmodule::test(SnortStore* snortstore)
{
	if(snortstore->is_valid){
		writer->writepacket(snortstore->get_record());
		delete snortstore;
	}
}


void Snortmodule::sigChild(int signum)
{
if (shutdown) return;
        
	msg(MSG_ERROR, "Snortmodule: External detection module died.");
        int status;
        pid_t pid = wait(&status);
        if (pid == -1)
                msg(MSG_ERROR, "Snortmodule: Can't determine exit state from external detecton module: %s", strerror(errno));

        if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                        msg(MSG_ERROR, "Snortmodule: Detection module with pid %i terminated normally.", pid);
                }else {
                        msg(MSG_ERROR, "Snortmodule: Detection module with pid %i terminated abnormally. Return value was: %i", pid, status);
                }
        } else {
                if (WIFSIGNALED(status)) {
                        msg(MSG_ERROR, "Snortmodule: Detection module with pid %i was terminated by signal %i", pid, WTERMSIG(status));
                }
        }
	shutdown=true;
  	CleanExit();
}



void Snortmodule::sigTerm(int signum)
{
	if (shutdown) return;
	shutdown=true;
	CleanExit();
}


void Snortmodule::sigInt(int signum)
{
	if (shutdown) return;
	shutdown=true;
	CleanExit();
}

#ifdef IDMEF_SUPPORT_ENABLED
void * Snortmodule::xmlWrapperEntry(void *args)
{
	wrapperConfig_t* config=(wrapperConfig_t* ) args;

	std::string message="";

	Snortmodule* object;
	object=(Snortmodule* )config->module;

	msg(MSG_INFO, "Snortmodule: xmlWrapper startup...");
   
	/* create FIFO */
	if ((mknod(config->fifoname.c_str(), S_IFIFO | 0666, 0)) < 0){
	      	msg(MSG_ERROR, "Snortmodule: Wrapper mknod failed");
		throw exceptions::DetectionModuleError("Snortmodule", "Can't create wrapper-FIFO", strerror(errno));
   	}
     
   	FILE * fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen(config->fifoname.c_str(), "r");
        if (fp == NULL){
		msg(MSG_ERROR, "Snortmodule: Wrapper FIFO open failed");
		throw exceptions::DetectionModuleError("Snortmodule", "Can't open wrapper-FIFO", strerror(errno));

	}
	    while ((read = getline(&line, &len, fp)) != -1) {
                 if ( (std::string)line == "</IDMEF-Message>\n"){
			 message+=line;
			 object->sendIdmefMessage(config->topic,message);
			 message="";
		 }else {
			 message+=line;
		 }
            }
        
    	unlink(config->fifoname.c_str());
	pthread_exit(NULL);
}
#endif
