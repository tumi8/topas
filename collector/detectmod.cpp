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

#include "detectmod.h"


#include <commonutils/global.h>
#include <concentrator/msg.h>


#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


DetectMod::DetectMod(const std::string& filename)
{
        this->filename = filename;
        /* Initial semahore key. We will try to find an unsed semaphore >= the initial value. */
        semKey = 10000;
        shmKey = 0;
	state = DetectMod::NotRunning;
}


DetectMod::~DetectMod()
{
}

void DetectMod::run() 
{
	state = DetectMod::Running;
        /* try to get an unused semaphore key */
        while ((-1 == (semId = semget(semKey, 1, S_IRWXU | IPC_CREAT | IPC_EXCL))) && errno == EEXIST) {
                ++semKey;
        }
        /* any other error creating the semaphore? */
        if (-1 == semId) {
                throw exceptions::DetectionModuleError(filename, "Can't create a semaphore for the detection module", strerror(errno));
        }

        /* create pipes */
        int tmpFd[2];
        if (-1 == pipe(tmpFd)) {
                throw exceptions::DetectionModuleError(filename, "Can't create pipes for the detection modules" , strerror(errno));
        }
        
        /* start the modules */
        if (-1 == (pid = fork())) {
                throw exceptions::DetectionModuleError(filename, "Can't fork a new process for starting the module", strerror(errno));
        }

        /* child process */
        if (pid == 0) {
                /* close writing side of pipe */
                if (-1 == close(tmpFd[1])) {
                        throw exceptions::DetectionModuleError(filename, "Could not close writing side of pipe",
                                                                 strerror(errno));
                }
                /* the detection modules read their input from stdin */
                if (dup2(tmpFd[0], STDIN_FILENO) != STDIN_FILENO) {
                        throw exceptions::DetectionModuleError(filename, "Could not dup pipe to stdin", strerror(errno));
                }

                if (-1 == close(tmpFd[0])) {
                        throw exceptions::DetectionModuleError(filename, "Could not close temporary pipe descriptor", strerror(errno));
                }

                /* build argument array */

                /* TODO: does the standard guarantee that std::string stores 0 terminated strings? */
                const char** args = (const char**)malloc((arguments.size() + 2) * sizeof(char*));
                *args = filename.c_str();
                for (unsigned i = 0; i != arguments.size(); ++i) {
                        *(args + i + 1) = arguments[i].c_str();
                }
                *(args + arguments.size() + 1) = 0;

                execv(filename.c_str(), (char* const*) args);
                throw exceptions::DetectionModuleError(filename, "Can't execute the detection module", strerror(errno));
        }

        /* parent process */

        /* store pipedeskriptor */
        if (-1 == close(tmpFd[0])) {
                throw exceptions::DetectionModuleError(filename, "Could not close reading side of pipe", strerror(errno));
        }
        if (-1 == (pipeFd = dup(tmpFd[1]))) {
                throw exceptions::DetectionModuleError(filename, "Could not dup temporary pipe descriptor", strerror(errno));
        }
        if (-1 == close(tmpFd[1])) {
                throw exceptions::DetectionModuleError(filename, "Could not close temporary pipe descriptor", strerror(errno));
        }
}

void DetectMod::stopModule() 
{
        kill(pid, SIGTERM);
        if (-1 == semctl(semId,0,IPC_RMID,NULL)) {
                msg(MSG_ERROR, "DetectMod: Error deleting semaphore to %s: %s", filename.c_str(), strerror(errno));
        }
}

void DetectMod::setShmKey(key_t s) 
{
        shmKey = s;
}

void DetectMod::setArgs(const std::vector<std::string>& args)
{
        arguments.clear();
        arguments.insert(arguments.begin(), args.begin(), args.end());
}

void DetectMod::restart()
{
        stopModule();
        run();
}

void DetectMod::restartCrashed()
{
        if (-1 == semctl(semId,0,IPC_RMID,NULL)) {
                msg(MSG_ERROR, "DetectMod: Error deleting semaphore to %s: %s", filename.c_str(), strerror(errno));
        } else {
                msg(MSG_INFO, "DetectMod: Successfully removed semaphore");
        }
        run();
}

void DetectMod::setState(State s)
{
	state = s;
}

DetectMod::State DetectMod::getState()
{
	return state;
}
