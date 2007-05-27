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

#ifndef _DATA_STORE_H_
#define _DATA_STORE_H_

#include <concentrator/rcvIpfix.h>
#include <concentrator/ipfix.h>
#include <concentrator/common.h>

#include <commonutils/msgstream.h>

#include <map>
#include <vector>
#include <stdexcept>

extern MsgStream msgStr;; // is defined in detectionbase.cpp

/**
 * Base class for all storage modules (You don't have to
 * derive your storage module from this class. But your
 * storage class must provide the functions
 * - void addFieldData(int id, byte* fieldData, int len);
 * - bool recordStart(SourceID&);
 * - void recordEnd();
 */
class DataStore 
{
 public:
        /**
         * Constructor 
         */
        DataStore() 
		: valid(false)
	{
	}

        /**
         * Destructor
         */
        ~DataStore() {}

        /**
         * @brief Method used to insert IPFIX data into the module
         * 
         * The method will be called for every field within an Ipfix
         * record.
         * The function will be called quite often.
         * @param id Field id (see concentrator/ipfix.h for a list of available ids
         * @param fieldData transmitted data
         * @param len Size of fieldData
         */
        void addFieldData(int id, byte* fieldData, int len, EnterpriseNo enterprise = 0) {}

        /**
         * Will be called whenever a new record starts.
         * This method can accept or reject a complete record.
         * DetectionBase will call @c recordEnd() before calling
         * recordStart() again.
         *
         * addFieldData() and recordEnd() will not be called, if
         * you reject the record.
         *
         * @return True for accepting the record, false otherwise
         */
        bool recordStart(SourceID) { return true; }

        /**
         * Indicates that all subscribed fields within a record where
         * passed to the module
         * Will only be called after a call to @c recordStart()
         */
        void recordEnd() {}
        

        /**
         * Transforms an IpfixField into an integer (host byte order)
         * @param n Field data to transform
         * @param length Length of field data
         */
       uint64_t fieldToInt(byte* n, unsigned length);

	/**
	 * Checkes wether the buffer is valid or not. A buffer may
	 * be invalid, if it is rejected by recordStart()
	 */
	bool isValid() { return valid; }
	
	/**
	 * Sets validity of the buffer element
	 */
	void setValid(bool v) { valid = v; }

private:
	bool valid;
};


#endif
