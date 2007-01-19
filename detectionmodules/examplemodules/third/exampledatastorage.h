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

#ifndef _EXAMPLE_DATA_STORAGE_H_
#define _EXAMPLE_DATA_STORAGE_H_

#include <map>
#include <vector>
#include <ostream>
#include <stdexcept>
#include <datastore.h>
#include <iostream>


class ExampleDataStorage : public DataStore 
{
 public:
        ExampleDataStorage();
        ~ExampleDataStorage();
        
        /**
         * Inserts the field with fieldId id into the storage class.
         */
        void addFieldData(int id, byte* fieldData, int fieldDataLength, EnterpriseNo eid = 0);
        
	bool recordStart(SourceID);
	void recordEnd();
	
	const IpAddress& getSourceIP() { return sourceAddress; }
	const IpAddress& getDestinationIP() { return destinationAddress; }

	uint16_t sourcePort;
	uint16_t destinationPort;
        
 private:
	IpAddress sourceAddress;
	IpAddress destinationAddress;
	bool recordStarted;
	int records;
};

#endif
