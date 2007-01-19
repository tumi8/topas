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

// 4 bytes - ip address
// 1 bytes - bits for subnet mask
// 2 bytes - port
class IpAndPort 
{
public:
        byte data[7]; 
        bool operator<(const IpAndPort& other) const 
        {
                return memcmp(data, other.data, 7)<0?true:false;
        }
};



class ExampleDataStorage : public DataStore 
{
 public:
        typedef std::map<IpAndPort, int> ExampleData;

        ExampleDataStorage();
        ~ExampleDataStorage();
        
        /**
         * Will be invoked, whenever a new data record starts
         * Will be used by @c DetectionBase
         */
        bool recordStart(SourceID);

        /**
         * Will be invoke, whenever a data record ends
         * Will be used by DetectionBase
         */
        void recordEnd();

        /**
         * Inserts the field with fieldId id into the storage class
         * Will be used by DetectionBase
         */
        void addFieldData(int id, byte* fieldData, int fieldDataLength, EnterpriseNo eid = 0);


        
        /**
         * Will NOT be used by DetectionBase
         */ 
        ExampleData::const_iterator begin() 
        {
                return addresses.begin();
        }

        /**
         * Will NOT be used by DetectionBase
         */
        ExampleData::const_iterator end() 
        {
                return addresses.end();
        }


        /*
         * Returns size of data vector
         * Will NOT be used by DetectionBase
         * @return size of data vector
         */
        unsigned size() 
        {
                return addresses.size();
        }

 private:
        ExampleData addresses;

        bool recordStarted;
        IpAndPort ipAndPort;
        bool gotIp, gotPort;
        int packetCount;
};

#endif
