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

#include "exampledatastorage.h"


ExampleDataStorage::ExampleDataStorage() 
	: sourceAddress(0,0,0,0), destinationAddress(0,0,0,0), sourcePort(0), destinationPort(0)
{
	recordStarted = false;
	records = 0;
}

ExampleDataStorage::~ExampleDataStorage() 
{
        /* do nothing */
}

void ExampleDataStorage::addFieldData(int id, byte* fieldData, int fieldDataLength, EnterpriseNo eid) 
{
	// only record 4 byte ip addresses
	if (id == IPFIX_TYPEID_sourceIPv4Address) {
		//std::cout << "read new source ipv4 address" << std::endl;
		sourceAddress.setAddress(fieldData);
	} else if (id == IPFIX_TYPEID_destinationIPv4Address) {
		//std::cout << "read new destination ipv4 address" << std::endl;
		destinationAddress.setAddress(fieldData);
	} else if (id == IPFIX_TYPEID_sourceTransportPort) {
		//std::cout << "read new source Port" << std::endl;
		bcopy(fieldData, &sourcePort, fieldDataLength);
	} else if (id == IPFIX_TYPEID_destinationTransportPort) {
		//std::cout << "read new destination Port" << std::endl;
		bcopy(fieldData, &destinationPort, fieldDataLength);
	}
}

bool ExampleDataStorage::recordStart(SourceID)
{
	//std::cout << "new record started" << std::endl;
	if (recordStarted) {
		std::cerr << "ExampleDataStorage::recordStart() was called while having a started record" << std::endl;
	}
	return recordStarted = true;
}

void ExampleDataStorage::recordEnd()
{
	//std::cout << "record ended" << std::endl;
	if (!recordStarted) {
		std::cerr << "ExampleDataStorage::recordEnd() was called before starting a record" << std::endl;
	}
	++records;
	if (records > 1) {
		std::cerr << "More than one record in ExampleDataStorage!!!!!!" << std::endl;
	}
}
