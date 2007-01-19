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

#include "mutex.h"


#include <concentrator/msg.h>


#include <cstring>


Mutex::Mutex()
{
	sem_init(&mutex, 0, 1);
}


Mutex::~Mutex()
{
	unlock();
	sem_destroy(&mutex);
}


void Mutex::lock()
{
	int ret = sem_wait(&mutex);
	if (ret != 0) {
		msg(MSG_ERROR, "Mutex: Error locking mutex: %s" , strerror(ret));
	}
}


void Mutex::unlock()
{
        int ret = sem_post(&mutex);
        if ( ret != 0 ) {
                msg(MSG_ERROR, "Mutex: Error unlocking mutex: %s", strerror(ret));
        }
}


bool Mutex::tryLock()
{
	if (0 == sem_trywait(&mutex)) 
		return true;
	return false;
}
