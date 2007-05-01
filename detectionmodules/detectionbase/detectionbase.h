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

#include "filepolicy.h"


#include <commonutils/sharedobj.h>
#include <commonutils/global.h>
#include <commonutils/idmef/idmefmessage.h>
#include <commonutils/confobj.h>
#include <concentrator/ipfix.h>
#include <concentrator/rcvIpfix.h>


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
        typedef enum {
                RUN,
                EXIT,
                RESTART
        } State;
        /**
         * Constructor taking path to configuration file. This configuration file is needed to
         * pass information about xmlBlaster sites to the module
         */
        DetectionBase(const std::string& configFile = "")
#ifdef IDMEF_SUPPORT_ENABLED
                : currentMessage(NULL), alarmTime(10)
#else
                : alarmTime(10)
#endif
        {
		confObj = new XMLConfObj(configFile, XMLConfObj::XML_FILE);

#ifdef IDMEF_SUPPORT_ENABLED
                /* parse confiuration file to get all xmlBlasters and their properties */
                if (confObj->nodeExists(config_space::XMLBLASTERS)) {
			confObj->enterNode(config_space::XMLBLASTERS);
			if (confObj->nodeExists(config_space::XMLBLASTER)) {
				confObj->setNode(config_space::XMLBLASTER);
				unsigned int count = 0;
				while (confObj->nextNodeExists()) {
					confObj->enterNextNode();
				 	/* Property does handle properties in the java-way */
					Property::MapType propMap;
					std::vector<std::string> props;
					/* get all properties */
					if (confObj->nodeExists(config_space::XMLBLASTER_PROP)) {
						props.push_back(confObj->getValue(config_space::XMLBLASTER_PROP));
						while (confObj->nextNodeExists()) {
							props.push_back(confObj->getNextValue());
						}
					} else {
						std::cerr << "No <" << config_space::XMLBLASTER_PROP 
							  << "> statement in config file, using default values"
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
					/* get module id here */
					analyzerId = globalRef.getElement()->getInstanceId();
					analyzerId = std::string(analyzerId.begin() + analyzerId.find_last_of("/") + 1, analyzerId.end());
					xmlBlasters.push_back(globalRef);
 					confObj->leaveNode();
				}
			} else {
				std::cerr << "No <" << config_space::XMLBLASTER << "> statement in config file" << std::endl;
			}
			
                } else {
                        std::cerr << "No <" << config_space::XMLBLASTERS << "> statement in config file" << std::endl;
                }
                
                /* connect to all xmlBlaster servers */
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

                std::cin >> topasID;
#endif // IDMEF_SUPPORT_ENABLED
        }
        
        
        /**
         * Destructor...
         */
        virtual ~DetectionBase() 
        {
#ifdef IDMEF_SUPPORT_ENABLED
                for (unsigned i = 0; i != commObjs.size(); ++i) {
			std::string managerID = (*xmlBlasters[i].getElement()).getProperty().getProperty(config_space::MANAGER_ID);
			if (managerID == "") {
                                msg(MSG_INFO, ("Using default " + config_space::MANAGER_ID + " \""
                                               + config_space::DEFAULT_MANAGER_ID + "\"").c_str());
                                managerID = config_space::DEFAULT_MANAGER_ID;
                        }
			// erase subsribed topic
			commObjs[i]->erase(analyzerName + "-" + analyzerId);
			// notify manager about exiting
			commObjs[i]->publish("<exit oid='" + analyzerName + "-" + analyzerId + "'/>", managerID);
			// disconnect
                        commObjs[i]->disconnect();
			delete commObjs[i];
                }
#endif
                delete confObj;
        }



        /**
         * Adds a type id to the list. Only data corresponding to field ids on the list
         * will be stored by the class.
         * If the list is empty, all received data will be passed to the module
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
                state = RUN;
                pthread_create(&testThread, NULL,
			       DetectionBase<DataStorage, InputPolicy>::testThreadFunc, this);
		pthread_create(&workingThread, NULL,
			       DetectionBase<DataStorage, InputPolicy>::workThreadFunc, this);

		while (state == RUN) {
			usleep(500);
		}

		pthread_cancel(testThread);
		pthread_cancel(workingThread);

                if (state == RESTART)
                        return -1;
                else if (state == EXIT)
                        return 0;

                // we should never get here!
                throw new std::runtime_error("DetectionBase: unkown state!!!!!!!!!");
        }

	static void* workThreadFunc(void* detectionbase_) {
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

                DetectionBase<DataStorage, InputPolicy>* dbase = static_cast<DetectionBase<DataStorage, InputPolicy>*>(detectionbase_);

                while (state == RUN) {
                        inputPolicy.wait();
                        inputPolicy.importToStorage();
                        inputPolicy.notify();
                }
	}

        /**
         * Test-Thread function. This function will run the tests implemented by derived classes.
         */
        static void* testThreadFunc(void* detectionbase_) 
        {
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

                DetectionBase<DataStorage, InputPolicy>* dbase = static_cast<DetectionBase<DataStorage, InputPolicy>*>(detectionbase_);

		// when alarmtime > 0, buffering is used
		// otherwise each record is seperately passed to the test function
		while(state == RUN) {
			// what a ugly hack! substitute this with some 
			// saved state!!!!!
			if((unsigned testInterval = dbase->getAlarmTime()) > 0) {
				time_t testTime = time(NULL) + testInterval;
				
				while(testInterval > 0 && state == RUN) {
					time_t t = time(NULL);
					if (t > testTime) {
						msg(MSG_ERROR, "DetectionBase: Test function is too slow.");
					} else {
						sleep(testTime - t);
					}
					testInterval = dbase->getAlarmTime(); // may have changed
					testTime = testTime + testInterval;

#ifdef IDMEF_SUPPORT_ENABLED
					for (unsigned i = 0; i != dbase->commObjs.size(); ++i) {
						std::string ret = dbase->commObjs[i]->getUpdateMessage();
						if (ret != "") {
							try {
								XMLConfObj* confObj = new XMLConfObj(ret, XMLConfObj::XML_STRING);
								dbase->update(confObj);
								delete confObj;
							} catch (const exceptions::XMLException &e) {
								msg(MSG_ERROR, e.what());
								dbase->sendControlMessage("<result>Manager: " + std::string(e.what()) + "</result>");
							}
						}
					}				
#endif
					// get received data into the user data struct 
					dbase->test(inputPolicy.getStorage());
				}
			}

			while(dbase->getAlarmTime() == 0 && state == RUN) {
				DataStorage* d = inputPolicy.getStorage();
				if (d->isValid()) {
					dbase->test(d);
				}
#ifdef IDMEF_SUPPORT_ENABLED
 				for (unsigned i = 0; i != dbase->commObjs.size(); ++i) {
					std::string ret = dbase->commObjs[i]->getUpdateMessage();
					if (ret != "") {
						try {
							XMLConfObj* confObj = new XMLConfObj(ret, XMLConfObj::XML_STRING);
							dbase->update(confObj);
							delete confObj;
						} catch (const exceptions::XMLException &e) {
							msg(MSG_ERROR, e.what());
							dbase->sendControlMessage("<result>Manager: " + std::string(e.what()) + "</result>");
						}						
					}
 				}				
#endif
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
         *           You have to delete the memory allocated for the object.
         */
        virtual void test(DataStorage* ds) {};


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
         * Creates a new Message using the parameters specified on earlier call to
         * getNewIdmefMessage(const std::string& analyzerName, const std::string& classification)
         * or empty strings if getNewIdmefMessage(const std::string& analyzerName, const std::string& classification)
         * was not called before.
         * @return new IDMEF Message
         */
        IdmefMessage& getNewIdmefMessage()
        {
                delete currentMessage;
                currentMessage = new IdmefMessage(analyzerName, analyzerId, classification, IdmefMessage::ALERT);
		currentMessage->setAnalyzerAttr("", topasID, "", "");
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

	void sendControlMessage(const std::string& message)
	{
		for (unsigned i = 0; i != commObjs.size(); ++i) {
			std::string managerID = (*xmlBlasters[i].getElement()).getProperty().getProperty(config_space::MANAGER_ID);
			if (managerID == "") {
				managerID = config_space::DEFAULT_MANAGER_ID;
			}
			commObjs[i]->publish(message, managerID);
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

protected:
	/** 
         * Register function. This function should be called 
	 * in the init() function of each detection module.
         * It registers the module and subscribes for update messages.
         * @param analyzerName Name of the detection module.
         */
	void registerModule(const std::string& analyzerName)
	{
		this->analyzerName = analyzerName;
		/* send <Heartbeat> message to all xmlBlaster servers and subscribe for update messages */
		IdmefMessage* heartbeatMessage = new IdmefMessage(analyzerName, analyzerId, classification, IdmefMessage::HEARTBEAT);
		heartbeatMessage->setAnalyzerAttr("", topasID, "", "");
		for (unsigned i = 0; i != commObjs.size(); ++i) {
			std::string managerID = (*xmlBlasters[i].getElement()).getProperty().getProperty(config_space::MANAGER_ID);
			if (managerID == "") {
				msg(MSG_INFO, ("Using default " + config_space::MANAGER_ID + " \"" 
					       + config_space::DEFAULT_MANAGER_ID + "\"").c_str());
				managerID = config_space::DEFAULT_MANAGER_ID;
			}
			heartbeatMessage->publish(*commObjs[i], managerID);
			commObjs[i]->subscribe(analyzerName + "-" + analyzerId, XmlBlasterCommObject::MESSAGE);
		}
		delete heartbeatMessage;
	}

	/**
	 * Returns topasID.
	 */
	std::string getTopasId()
	{
		return topasID;
	}
	
	/**
	 * Returns moduleID.
	 */
	std::string getModuleId()
	{
		return analyzerName + "-" + analyzerId;
	}

	/** 
         * Update function. This function will be called, whenever a message
         * for subscribed key is received from xmlBlaster.
         * _NEVER_ CALL THIS FUNCTION BY HAND IN AN DERIVED CLASS
         * @param xmlObj Pointer to data structure, containing xml data
         *               You have to delete the memory allocated for the object.
         */
	virtual void update(XMLConfObj* xmlObj) = 0;

#endif // IDMEF_SUPPORT_ENABLED

	/**
	 * Restarts the module.
	 */
	static void restart() {
		state = RESTART;
	}
	
	/**
	 * Stops the module.
	 */
	static void stop() {
		state = EXIT;
	}


	
private:
#ifdef IDMEF_SUPPORT_ENABLED
        IdmefMessage* currentMessage;
        std::string analyzerName, analyzerId, classification;
        std::string topasID;
        std::vector<XmlBlasterCommObject*> commObjs;
        std::vector<GlobalRef> xmlBlasters;
#endif
        static InputPolicy inputPolicy;

        pthread_t testThread;
	pthread_t workingThread;
        static volatile State state;

        XMLConfObj* confObj;

        unsigned alarmTime;
};



template<class DataStorage, class InputPolicy>
InputPolicy DetectionBase<DataStorage, InputPolicy>::inputPolicy;
template<class DataStorage, class InputPolicy>
volatile typename DetectionBase<DataStorage, InputPolicy>::State
	DetectionBase<DataStorage, InputPolicy>::state = 
		(typename DetectionBase<DataStorage, InputPolicy>::State)0;

#endif
