/**************************************************************************/
/*    Copyright (C) 2005-2007 Lothar Braun <mail@lobraun.de>              */
/*         2007 Raimondas Sasnauskas <sasnausk@informatik.uni-tuebigen.de */
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

#ifndef _DETECTION_BASE_H_
#define _DETECTION_BASE_H_

/**
 * @author Lothar Braun <braunl@informatik.uni-tuebingen.de>
 */


#include "filepolicy.h"


#include <commonutils/sharedobj.h>
#include <commonutils/global.h>
#include <commonutils/mutex.h>
#include <commonutils/idmef/idmefmessage.h>
#include <commonutils/confobj.h>
#include <concentrator/ipfix.h>
#include <concentrator/rcvIpfix.h>


#include <signal.h>


#include <fstream>

#include <sstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <cstring>



#include <stdexcept>
#include <vector>


/**
 * Base class for all detection modules used within the IDS.
 * The base class hides all IPFIX stuff from the detection module
 * authors.
 */
template <
        class DataStorage,
        class InputPolicy = BufferedFilesInputPolicy<SemShmNotifier, DataStorage>
>
class DetectionBase 
{
 public:
        /**
         * Constructor
         * @deprecated
         */
        DetectionBase()
#ifdef IDMEF_SUPPORT_ENABLED
                : currentMessage(NULL), confObj(NULL), alarmTime(10)
#else
                : confObj(NULL), alarmTime(10)
#endif
        {
		testMutex.lock();
                
                if (SIG_ERR == signal(SIGALRM, DetectionBase<DataStorage, InputPolicy>::sigAlarm)) {
                        std::cerr << "Can't install signal handler for SIGALARM: " << strerror(errno) << std::endl;
                        throw std::runtime_error("Could not install signal handler for SIGALARM");
                }
                if (SIG_ERR == signal(SIGINT, SIG_IGN)) {
                        std::cerr << "Can not ignore SIGINT, this will maybe result in strange error messages, when shuting the"
                                  << "down the system with STRG+C" << std::endl;
                }
        }

        /**
         * Constructor taking path to configuration file. This configuration file is needed to
         * pass information about xmlBlaster sites to the module
         */
        DetectionBase(const std::string& configFile)
#ifdef IDMEF_SUPPORT_ENABLED
                : currentMessage(NULL), alarmTime(10)
#else
                : alarmTime(10)
#endif
        {
		testMutex.lock();
                
                if (SIG_ERR == signal(SIGALRM, DetectionBase<DataStorage, InputPolicy>::sigAlarm)) {
                        std::cerr << "Could not install signal handler for SIGALARM: " << strerror(errno) << std::endl;
                        throw std::runtime_error("Could not install signal handler for SIGALARM");
                }
                /* parse confiuration file to get all xmlBlasters and their properties */
                confObj = new XMLConfObj(configFile);
#ifdef IDMEF_SUPPORT_ENABLED
                if (confObj->nodeExists("xmlBlasters")) {
			confObj->enterNode("xmlBlasters");
			if (confObj->nodeExists("xmlBlaster")) {
				confObj->setNode("xmlBlaster");
				unsigned int count = 0;
				while (confObj->nextNodeExists()) {
					confObj->enterNextNode();
				 	/* Property does handle properties in the java-way */
					Property::MapType propMap;
					std::vector<std::string> props;
					/* get all properties */
					if (confObj->nodeExists("prop")) {
						props.push_back(confObj->getValue("prop"));
						while (confObj->nextNodeExists()) {
							props.push_back(confObj->getNextValue());
						}
					} else {
						std::cerr << "No <prop> statement in config file, using default values"
							  << std::endl;
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
					xmlBlasters.push_back(globalRef);
 					confObj->leaveNode();
				}
			} else {
				std::cerr << "No <xmlBlaster> statement in config file" << std::endl;
			}

                } else {
                        std::cerr << "No <xmlBlasters> statement in config file" << std::endl;
                }
                
                /* connect to all xmlBlaster sites */
                for (unsigned i = 0; i != xmlBlasters.size(); ++i) {
			try {
				XmlBlasterCommObject* comm = new XmlBlasterCommObject(*xmlBlasters[i].getElement());
				comm->connect();
				commObjs.push_back(comm);
			} catch (const XmlBlasterException &e) {
				std::cerr << "Cannot connect to xmlBlaster: " << std::endl
					  << e.toXml() << std::endl
					  << "Make sure, the xmlBlaster server is up and running." 
					  << std::endl;
			        throw std::runtime_error("Cannot connect to xmlBlaster: \n" + e.toXml()
							 + "\nMake sure, the xmlBlaster server is up and running.");
			}                                    
                }

#endif // IDMEF_SUPPORT_ENABLED
        }
        
        
        /**
         * Destructor...
         */
        virtual ~DetectionBase() 
        {
#ifdef IDMEF_SUPPORT_ENABLED
                delete currentMessage;
                for (unsigned i = 0; i != commObjs.size(); ++i) {
                        commObjs[i]->disconnect();
			delete commObjs[i];
                }
#endif
                delete confObj;
        }



        /**
         * Adds a type id to the list. Only data corresponding to field ids on the list
         * will be stored by the class.
         * If the list is empty, all data will be stored
         * @param id Field ID to be stored in the list
         */
        void subscribeTypeId(int id)  
        {
                inputPolicy.subscribeId(id);
        }

	/**
	 * Adds a given source id to the list of allowed source ids.
	 * Only packets which contain one of the subscribed source ids,
	 * will be passed to the detection algorithms. If the module author
	 * does not call this function, all incoming IPFIX-Packets will
	 * be passed to the module's detection algorithm.
	 */
	void subscribeSourceId(uint16_t id)
	{
		inputPolicy.subscribeSourceId(id);
	}
        

        /**
         * Main routing of the detectionbase. This function is responsible for collecting data from the files
         */
        int exec() 
        {
                createTestThread();
                for (;;) {
                        inputPolicy.wait();
                        inputPolicy.importToStorage();
                        inputPolicy.notify();
                }
                return 0;
        }


        /**
         * Sets the new interval, after which a new test should be performed. Changes to the alarmtime
	 * will take effect after the currently running alarm is triggered.
         * @param sec Seconds till next test run
         */
        void setAlarmTime(unsigned  sec) 
        {
                alarmTime = sec;
        }


        /**
         * Returns time, after which the test should be started.
         * @return Time in seconds, after which the test should be started.
         */
        unsigned getAlarmTime() { return alarmTime; }

#ifdef IDMEF_SUPPORT_ENABLED
        /**
         * Returns the current IDMEF message object. A new message is generated if there is no current message.
         * Uses the parameters specified on earlyer call to getNewIdmefMessage(const std::string& analyzerName, const std::string& classification)
         * or empty strings if getNewIdmefMessage(const std::string& analyzerName, const std::string& classification)
         * was not called before.
         * @return IDMEF message object.
         */
        IdmefMessage& getCurrentIdmefMessage()
        {
                if (currentMessage == NULL) {
                        return getNewIdmefMessage();
                }
                return *currentMessage;
        }

        /**
         * Creates a new Message using the parameters specified on earlyer call to
         * getNewIdmefMessage(const std::string& analyzerName, const std::string& classification)
         * or empty strings if getNewIdmefMessage(const std::string& analyzerName, const std::string& classification)
         * was not called before.
         * @return new IDMEF Message
         */
        IdmefMessage& getNewIdmefMessage()
        {
                delete currentMessage;
                currentMessage = new IdmefMessage(analyzerName, classification);
                return *currentMessage;
        
        }

        /**
         * Creates and returns a new IdmefMessage. Pointer to older messages will get invalidated
         * @param analyzerName Name of the detection module.
         * @param classification Classification of the detection method.
         * @return new Idmef message 
         */
        IdmefMessage& getNewIdmefMessage(const std::string& analyzerName, const std::string& classification)
        {
                this->analyzerName = analyzerName;
                this->classification = classification;
                return getNewIdmefMessage();
        }


        /**
         * Sends message to all configured XmlBlasters
         * @param topic Publish the IDMEF-Message under given topic..
         * @param idmefMessage IDMEF-Message to send
         */
        void sendIdmefMessage(const std::string& topic, IdmefMessage& idmefMessage)
        {
                for (unsigned i = 0; i != commObjs.size(); ++i) {
                        idmefMessage.publish(*commObjs[i], topic);
                }
        }

	void sendIdmefMessage(const std::string& topic, const std::string& message)
	{
		for (unsigned i = 0; i != commObjs.size(); ++i) {
			commObjs[i]->publish(message, topic);
		}
	}

        /**
         * Sends current IDMEF-Message
         * @param topic Publish the IDMEF-Message under given topic..
         */
        void sendIdmefMessage(const std::string& topic)
        {
                sendIdmefMessage(topic, *currentMessage);
        }
#endif //IDMEF_SUPPORT_ENABLED

 protected:

        /**
         * Test-Thread function. This function will run the tests implemented by derived classes.
         */
        static void* testThreadFunc(void* detectionbase_) 
        {
                DetectionBase<DataStorage, InputPolicy>* dbase = static_cast<DetectionBase<DataStorage, InputPolicy>*>(detectionbase_);
		// when alarmtime > 0, buffering is used
		// otherwise each record is seperately passed to the test function
		while(1) {
			// what a ugly hack! substitute this with some 
			// saved state!!!!!
			while(dbase->getAlarmTime() > 0) {
				alarm(dbase->getAlarmTime());
				dbase->testMutex.lock();
				// get received data into the user data struct 
				dbase->test(inputPolicy.getStorage());
			}
			while(dbase->getAlarmTime() == 0) {
				DataStorage* d = inputPolicy.getStorage();
				if (d->isValid())
					dbase->test(d);
			}
		}
                
                return NULL;
        }

        /** 
         * Test function. This function will be called, whenever its time to do the test.
         * You should override this function in derived classes.
         * _NEVER_ CALL THIS FUNCTION BY HAND IN AN DERIVED CLASS
         * @param ds Pointer to data structure, containing all data collected since last call
         *           to test(). 
         *           You have to delete the memeoty allocated for the object.
         */
        virtual void test(DataStorage* ds) = 0;


 private:
        static InputPolicy inputPolicy;

        pthread_t testThread;

        /**
         * If test_mutex locked, no test will be performed
         */
	static Mutex testMutex;
#ifdef IDMEF_SUPPORT_ENABLED
        IdmefMessage* currentMessage;
        std::string analyzerName, classification;
        std::vector<XmlBlasterCommObject*> commObjs;
        std::vector<GlobalRef> xmlBlasters;
#endif
        XMLConfObj* confObj;


        unsigned alarmTime;


        /**
         * Create an thread wich will be responsible to start the test
         * after the configured interval. 
         */
        void createTestThread() 
        {
                pthread_create(&testThread, NULL, DetectionBase<DataStorage, InputPolicy>::testThreadFunc, this);
        }

        
        /**
         * Signal handler for SIGALRM. This signal is emmited, evry time
         * a new test should be performed
         */
        static void sigAlarm(int) 
        {
		testMutex.unlock();
        }
};



template<class DataStorage, class InputPolicy>
Mutex DetectionBase<DataStorage, InputPolicy>::testMutex;
template<class DataStorage, class InputPolicy>
InputPolicy DetectionBase<DataStorage, InputPolicy>::inputPolicy;


#endif
