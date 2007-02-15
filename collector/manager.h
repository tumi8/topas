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

#ifndef MANAGER_H
#define MANAGER_H


/**
 * @author Lothar Braun <braunl@informatik.uni-tuebingen.de>
 */


#include "modulecontainer.h"


#include <commonutils/global.h>
#include <commonutils/mutex.h>
#include <commonutils/confobj.h>
#include <commonutils/idmef/idmefmessage.h>

#include <string>
#include <vector>

class collector;
class DetectModExporter;


/**
 * The manager maintains a list of detection modules, informs them about incoming IPFIX packets and deletes the new packets
 * after they were parsed by the modules. The class is also responsible for starting and controlling the detection modules.
 */
class manager{
public:
        /**
         * Default constructor
         */
        manager(DetectModExporter* exporter);

        /**
         * Destructor
         */
        ~manager();
    
        /**
         * Adds a detection module to the list maintained by the manager. The detection module will be started
         * and will be informed about new incoming IPFIX packets.
         * @param module_path Path to the executable of the detection module. The file must have the execution bit set.
	 * @param arguments Arguments that are passed to the module on startup.
         */
        void addDetectionModule(const std::string& module_path, const std::vector<std::string>& arguments);

        
        /**
         * The manager is informed by the collector about the arrival of new packets via this function.
         */
        void newPacket();


        /**
         * Locks the mutex.
         */
        void lockMutex();

        /**
         * Unlocks the mutex. 
         */
        void unlockMutex();


        /**
         * This method forks the detection modules, and sets up communication devices to them (shared memory and semaphores)
         */
        void startModules();

        /**
         * Informs manager that the system is going to be shut down soon. After calling this mehtod, the manager won't care about
         * dying detection modules any more.
         */
        static void prepareShutdown();

	/**
	 * Kills all running modules. Call @c prepareShutdown() if you want to surpress the restarting algorithm.
	 */
	void killModules();


private:
        static ModuleContainer detectionModules;
        static DetectModExporter* exporter;

        unsigned killTime;
        static bool restartOnCrash;
        static bool shutdown;

	Mutex mutex;

        /**
         * Signal handler for the signal SIGALRM
         */
        static void sigAlarm(int);

        /**
         * Signal handler for the signal SIGCHLD
         */
        static void sigChild(int);

#ifdef IDMEF_SUPPORT_ENABLED
        /**
         * Update function. This function will be called, whenever a message
         * for subscribed key is received from xmlBlaster.
         * @param xmlObj Pointer to data structure, containing xml data
         *               You have to delete the memory allocated for the object.
         */
        void update(XMLConfObj* xmlObj);
#endif

protected:
        /**
         * The main manager thread function. This function may only be started by the @c collector
         */
        static void* run(void*);
        
        /** @c collector */
        friend class collector;

#ifdef IDMEF_SUPPORT_ENABLED
        /** TOPAS id for XMLBlaster */
        std::string topasID;
        std::vector<XmlBlasterCommObject*> commObjs;
        std::vector<GlobalRef> xmlBlasters;
#endif
};


#endif
