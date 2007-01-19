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


#include <cassert>


ExampleDataStorage::ExampleDataStorage() 
{
        recordStarted = false;
        gotIp = false;
        gotPort = false;
        packetCount = 1;
}

ExampleDataStorage::~ExampleDataStorage() 
{
}

void ExampleDataStorage::addFieldData(int id, byte* fieldData, int fieldDataLength, EnterpriseNo eid) 
{
        switch (id) {
        case IPFIX_TYPEID_destinationIPv4Address:
                /* fieldDataLength == 5 means fieldData == xxx.yyy.zzz/aa (ip address and netmask)
                   fieldDataLength == 4 means fieldData == xxx.yyy.zzz/0  (ip address)
                */
                switch (fieldDataLength) {
                case 5:
                        memcpy(ipAndPort.data, fieldData, 5);
                        gotIp = true;
                        break;
                case 4:
                        memcpy(ipAndPort.data, fieldData, 4);
                        ipAndPort.data[4] = 0;
                        gotIp = true;
                        break;
                default:
                        std::cerr << "Got ip address with unparsable length" << std::endl;
                }
                break;
        case IPFIX_TYPEID_destinationTransportPort:
                if (fieldDataLength == 2) {
                        uint16_t tmp = ntohs(*(uint16_t*)fieldData);
                        *(uint16_t*)(ipAndPort.data + 5) = tmp;
                        gotPort = true;
                }
                break;
        case IPFIX_TYPEID_packetDeltaCount:
                packetCount = fieldToInt(fieldData, fieldDataLength);
                break;
        default:
                break;
        }
}


bool ExampleDataStorage::recordStart(SourceID id) 
{
        assert(recordStarted == false);
        recordStarted = true;
        gotIp = false;
        gotPort = false;
        packetCount = 1;
        return true;
}

void ExampleDataStorage::recordEnd() 
{
        assert(recordStarted == true);
        
        if (gotIp && gotPort)
                addresses[ipAndPort] += packetCount;
        
        recordStarted = false;
}
