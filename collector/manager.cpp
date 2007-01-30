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

	mutex.lock();
}


manager::~manager()
{
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

        msg(MSG_ERROR, "Manager: A detection module died.");
        int status;
        pid_t pid = wait(&status);
        if (pid == -1)
                msg(MSG_ERROR, "Manager: Can't determine exit state from "
		    "detecton module: %s", strerror(errno));

        if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                        msg(MSG_ERROR, "Manager: Detection module with pid %i"
			    "terminated normally. Not restarting module", pid);
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
                
                // TODO: check and handle XML-Blaster messages

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
