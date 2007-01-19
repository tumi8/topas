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

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <semaphore.h>


/**
 * Create and handle a mutex
 *
 * This class works like a mutex which can be aquired through pthread_mutex_create.
 * We use pthread-semaphores because of incompability between linux pthread_mutexes and
 * POSIX pthread-mutex (as implemented by FreeBSD).
 * The Linux pthread implementation allows to unlock a mutex wich was locked by
 * another thread. POSIX doesn't .... :(
 * I therefore implemented this class with pthread-semaphores to bypass this restriction.
 */
class Mutex {
public:
	/**
	 * Creates a new mutex. Default state is unlocked
	 */
	Mutex();

	/**
	 * Destroyes the mutex. Unlocks it before deletion.
	 */
	~Mutex();

	/**
	 * Locks the mutex.
	 */
	void lock();

	/**
	 * Unlocks the mutex.
	 */
	void unlock();

	/**
	 * Tries to lock the mutex. This is a non-blocking mutex operation
	 * @return true if mutex could be locked immediately. False otherwise.
	 */
	bool tryLock();

private:
	sem_t mutex;
};

#endif 
