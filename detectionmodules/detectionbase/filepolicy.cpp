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

#include "filepolicy.h"


#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


#include <iostream>



SemShmNotifier::SemShmNotifier() 
{
	std::string tmp;
        std::cin >> semKey >> shmKey >> tmp;
	
	if (tmp == "USE_FILES") {
		useFiles_ = true;
	} else {
		useFiles_ = false;
	}
	
	std::cin >> packetDir;

	std::cout << "\"" << packetDir << "\"" << std::endl;

        if (-1 == (semId = semget(semKey, 0, 0))) {
                std::cerr << "Could not open semaphore:" << strerror(errno) << std::endl;
                throw std::runtime_error("Could not open semaphore");
        }
        
	
        nps = new shared::SharedObj(shmKey);
	
	if (!useFiles_) {
		int id = shmget(nps->getStorageKey(), nps->getStorageSize(), S_IRWXU);
		if (-1 == id) {
			throw std::runtime_error("Could not get shm id!");
		}
		void* ptr = shmat(id, NULL, SHM_RDONLY);
		if ((void*)-1 == ptr) {
			throw std::runtime_error(std::string("SemShmNotifier: Could not attach shared memory storage area: ") + strerror(errno));
			
		}
		
		IpfixShm::setShmPointer((byte*)ptr);
		IpfixShm::setShmSize(nps->getStorageSize());
	}
}


SemShmNotifier::~SemShmNotifier()
{

}

int SemShmNotifier::wait() const
{
        /* wait for incoming packet */
        struct sembuf semaphore;
	bool tryAgain = false;
	do {
        	semaphore.sem_num = 0;
	        semaphore.sem_op = -1;
	        semaphore.sem_flg = 0;
	        if (-1 == semop(semId, &semaphore, 1)) {
			// where we interupted by a signal?
			if (errno == EINTR) {
				tryAgain = true;
			} else {
	                	std::cerr << "Detection Modul (SemShmNotifier::wait()): Error decrementing the semaphore: " 
					  << strerror(errno) << std::endl;
				tryAgain = false;
			}
	        } else {
			tryAgain = false;
		}
	} while (tryAgain);
        return 1;
}

int SemShmNotifier::notify() const
{
        struct sembuf semaphore;

        // decrement semaphore
        semaphore.sem_num = 0;
        semaphore.sem_op = -1;
        semaphore.sem_flg = 0;
        if (-1 == semop(semId, &semaphore, 1)) {
                std::cerr << "Detection Modul (SemShmNotifier::notify()): Error decrementing the semaphore: " 
                          << strerror(errno) << std::endl;
        }
        return 0;
}
