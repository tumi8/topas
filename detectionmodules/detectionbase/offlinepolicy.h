/**************************************************************************/
/*    Copyright (C) 2007 Gerhard Muenz                                    */
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

#ifndef _OFFLINE_POLICY_H_
#define _OFFLINE_POLICY_H_


#include "inputpolicybase.h"

#include <fstream>
#include <iostream>


/** 
 * Reads storage data from file and returns them into a storage object. 
 */ 
template <
	class Storage
>
class OfflineInputPolicy : public InputPolicyBase<InputNotificationBase, Storage> {
public:
	OfflineInputPolicy() {
		dataAvailableLock.lock();
	}

	~OfflineInputPolicy() {
	    if(inputstr.is_open())
		inputstr.close();
	}

	/**
	 * Sets input file name. 
	 * @returns returns false if operation failed
	 */
	static bool openOfflineFile(const char* filename)
	{
	    inputstr.open(filename);
	    if(!inputstr)
		return false;
	    return true;
	} 
		
	/**
	 * Checks if file is opened and data is available.
	 * @returns returns 1 if more data is available, 0 otherwise
	 */
	int wait()
	{
	    bufferLock.lock();
	    if(inputstr.is_open() && !inputstr.eof())
		return 1;
	    return 0;
	}
	    
	/**
	 * Creates new storage object and reads data from file.
	 */
	void importToStorage() 
	{
                buffer = new Storage();
		inputstr >> buffer;
		if(!(!inputstr))
		    buffer->setValid(true);
		dataAvailableLock.unlock();
	}

	/**
	 * Returns storage if importToStorage has been called. Blocks otherwise.
	 */
        Storage* getStorage()
        {
		Storage* ret;
		dataAvailableLock.lock();
		ret = buffer;
		bufferLock.unlock();
		return ret;
        }

private:
	Storage* buffer;
	static std::ifstream inputstr;
	Mutex bufferLock;
	Mutex dataAvailableLock;
};

template <class Storage> std::ifstream OfflineInputPolicy<Storage>::inputstr;

#endif
