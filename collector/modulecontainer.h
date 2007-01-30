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

#ifndef _MODULE_CONTAINER_H_
#define _MODULE_CONTAINER_H_


#include "detectmod.h"


#include <sys/types.h>


#include <vector>
#include <string>


class DetectModExporter;


/**
 * @class
 * Provides an threadsafe container for detection modules. This container ensures that a crashed detection
 * module can be savely deleted/restarted
 */
class ModuleContainer
{
public:
        /**
         * Constructor...
         */
        ModuleContainer();

        /**
         * Destructor...
         */
        ~ModuleContainer();

	/**
	 * Creates new detection module
	 * @param command path to the detection modules binary
	 * @param args arguments passed to the detection modules
	 */
	void createModule(const std::string& command, const std::vector<std::string>& args);
               

        /**
         * Restart module with given pid
         * @param pid Pid of crashed module
         */
        void restartCrashedModule(pid_t pid, DetectModExporter* exporter);

        /**
         * Removes detection module with pid pid from the list of stored and managed detectio modules
         * @praram pid pid of detection module to remove
         */
        void deleteModule(pid_t pid);


        /**
         * Kills all started detection modules.
         */
        void killDetectionModules();

        /**
         * This method forks the detection modules, and sets up communication devices to them (shared memory and semaphores)
         * @param exporter Every module has to be associated to an DetectModExporter. This is the exporter all modules
         *                 should attach to.
         */
        void startModules(DetectModExporter* exporter);

	/**
	 * Looks for Module with given pid and changes state of module.
	 * @param pid Process id of module.
	 * @param state New state.
	 */
	void setState(pid_t pid, DetectMod::State);

	/**
	 * Informes all modules that new data arrived. This method will return, as soon as every (active) module
	 * processed the new data.
	 * @param exporter Exporter instance used to inform the modules.
	 */
	void notifyAll(DetectModExporter* exporter);

private:
        std::vector<DetectMod*> detectionModules;
};

#endif
