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

#ifndef DETECTMOD_H
#define DETECTMOD_H

/**
 * @author Lothar Braun <braunl@informatik.uni-tuebingen.de>
 */


#include <sys/types.h>


#include <string>
#include <vector>


/**
 * The detectMod class handles all information concerning the detection modules started by the manager
 */
class DetectMod
{
public:
	enum State {
		NotRunning,
                Remove,
		Running,
		Crashed
	};

        /**
         * Constructor. Constructs the detectMod object.
         * @param filename Filename (including path) of the detection module.
         */
        DetectMod(const std::string& filename);

        /**
         * Destructor.
         */
        ~DetectMod();
        
        /**
         * run() forks a new process and starts the detection module.
         */
        void run();

        /**
         * Restarts a running detectionmodule. This does mean, that DetectMod tries to kill
         * the process with the stored pid.
         */
        void restart();

        /**
         * Restarts a running detectionmodule. The existing process won't be killed before starting a
         * new instance
         */
        void restartCrashed();


        /**
         * Binds a shared memory block
         */
        void setShmKey(key_t shm_key);

        /**
         * Stops a running module. Doesn't do anything if module isn't started yet
         */
        void stopModule();
        
        /**
         * Returns the pid of detection module.
         * @return Pid of the process
         */
        pid_t getPid() const { return pid; }

        /**
         * Returns the semaphore id of the semaphore needed to sync with the manager.
         * @return Semaphore id
         */
        int getSemId() const { return semId; }

        /**
         * Returns the filename of the started detection module.
         * @return Filename of the detection module.
         */
        const std::string getFileName() const { return filename; }

        /**
         * Returns semaphore key
         * @return semaphore key
         */
        key_t getSemKey() const { return semKey; }
        
        /**
         * Returns shared memory key
         * @return shared memory key
         */
        key_t getShmKey() const { return shmKey; }

        /**
         * Returns pipe descriptor
         * @return pipe descriptor
         */
        int getPipeFd() const { return pipeFd; }

        /**
         * Sets arguments to be passed to the module on startup.
         * A call to this method will override previously set arguments.
         */
        void setArgs(const std::vector<std::string>& args);

	/**
	 * Sets module state
	 * @param s new state
	 */
	void setState(State s);


	/**
	 * Get module state.
	 * @return Module state.
	 */
	State getState();

private:
        pid_t pid;
        std::string filename;
        key_t semKey;
        int semId;
        key_t shmKey;

        int pipeFd;

        std::vector<std::string> arguments;

	State state;
};


#endif
