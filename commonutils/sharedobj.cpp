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

#include "sharedobj.h"
#include "exceptions.h"


#include <concentrator/msg.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>


#include <cstring>
#include <stdexcept>
#include <iostream>

using namespace shared;

SharedObj::SharedObj() 
	: deleteBlock(true)
{
        /* look for a shm_key which is not in use */
        shmKey = 10000;
        while (-1 == (shmId = (shmget(shmKey, config_space::SHM_SIZE, S_IRWXU | IPC_CREAT | IPC_EXCL))) && errno == EEXIST) {
                ++shmKey;
        }
        if (shmId == -1) {
                msg(MSG_FATAL, "Shared_Obj: Could not allocate shared memory: %s\n", strerror(errno));
                throw exceptions::DetectionModuleError("SharedObj: Could not work without shared memory segment");
        }
        
        if ((void*)-1 ==  (sb = (ShmBlock*)shmat(shmId, NULL, 0))) {
                throw exceptions::DetectionModuleError("SharedObj", "Can't attach shared memory block", strerror(errno));
        }

}

SharedObj::SharedObj(key_t s) 
	: deleteBlock(false)
{
        shmKey = s;

        if (-1 == (shmId = shmget(shmKey, 0, 0))) {
                std::cerr <<  "Detection Module: Could not get shared memory id: " << strerror(errno) << std::endl;
                throw exceptions::DetectionModuleError("SharedObj", "Could not get identifier for the shared memory segment", strerror(errno)); 
        }
        
        if ((void*)-1 == (sb = (ShmBlock*)shmat(shmId, 0, SHM_RDONLY))) {
                std::cerr << "Detection Module: Could not attach shared memory block: " << strerror(errno) <<  std::endl;
                throw exceptions::DetectionModuleError("SharedObj", "Could not attach shared memory block", strerror(errno));
        }
}

SharedObj::~SharedObj() 
{
	if (deleteBlock) {
		if (-1 == shmctl(shmId, IPC_RMID, NULL)) {
			msg(MSG_ERROR, "Shared_Obj: Can't delete shared memory block: %s", strerror(errno));
		}
	}
}

/******************************************************************* file_counter *******************************************************************/


FileCounter::FileCounter(unsigned c) : i(c)
{
}

FileCounter::~FileCounter() 
{
}
                
// pre-increment
const FileCounter& FileCounter::operator++() 
{
        if (++i == config_space::MAX_FILES)
                i = 0;
        return *this;
}

// post-increment 
const FileCounter FileCounter::operator++(int) 
{
        FileCounter ret = FileCounter(*this);
        if (++i == config_space::MAX_FILES)
                i = 0;
        return ret;
}

// pre-decrement
const FileCounter& FileCounter::operator--() 
{
        if (--i == 0)
                i = config_space::MAX_FILES -  1;
        return *this;
}

// post-decrement
const FileCounter FileCounter::operator--(int) 
{
        FileCounter ret = FileCounter(*this);
        if (--i == 0)
                i = config_space::MAX_FILES - 1;
        return ret;
}


FileCounter FileCounter::operator+(const FileCounter& rhs) const 
{
        FileCounter ret;
        if ((ret.i = i + rhs.i) >= config_space::MAX_FILES) {
                ret.i -= config_space::MAX_FILES;
        }
        return ret;
}


FileCounter FileCounter::operator+(const unsigned& rhs) const 
{
        FileCounter ret;
        if ((ret.i = i + rhs) >= config_space::MAX_FILES)
                ret.i -= config_space::MAX_FILES;
        return ret;
}
                
FileCounter FileCounter::operator-(const FileCounter& rhs) const 
{
        FileCounter ret;
        ret.i = (i >= rhs.i)?(i-rhs.i):(config_space::MAX_FILES - (rhs.i - i));
        return ret;
}
                
FileCounter FileCounter::operator-(const unsigned& rhs) const 
{
        FileCounter ret;
        ret.i = (i >= rhs)?(i-rhs):(config_space::MAX_FILES - (rhs - i));
        return ret;
}

FileCounter& FileCounter::operator=(const unsigned& rhs) 
{
        if (rhs > config_space::MAX_FILES)
                throw std::range_error("Number to big");
        i = rhs;
        return *this;
}

FileCounter& FileCounter::operator=(const FileCounter& rhs) 
{
        i = rhs.i;
        return *this;
}

bool FileCounter::operator==(const FileCounter& rhs) const 
{
        if (i == rhs.i) 
                return true;
        return false;
}

bool FileCounter::operator!=(const FileCounter& rhs) 
{
        if (i != rhs.i)
                return true;
        return false;
}

bool FileCounter::operator!=(const unsigned& rhs) const 
{
        if (i != rhs)
                return true;
        return false;
}

FileCounter::operator unsigned () const
{
        return i;       
}

/******************************************************************* Friend functions **************************************************************/


std::ostream& shared::operator<<(std::ostream& os, const FileCounter& fc) 
{
        os << fc.i;
        return os;
}

