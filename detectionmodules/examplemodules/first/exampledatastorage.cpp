/**************************************************************************/
/*    Copyright (C) 2005-2007 Lothar Braun <mail@lobraun.de>              */
/*                            Gerhard Muenz                               */
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
{
        /* do nothing */
}

ExampleDataStorage::~ExampleDataStorage() 
{
        /* do nothing */
}

void ExampleDataStorage::addFieldData(int id, byte* fieldData, int fieldDataLength, EnterpriseNo eid) 
{
        // We subscribed only to the "Destination ip address" field (see ExampleModule::ExampleModule())
        // These ip addresses are stored in a vector
        addresses.push_back(IpAddress(fieldData[0], fieldData[1],
                                      fieldData[2], fieldData[3]));
}

std::ofstream& operator<<(std::ofstream& os, const ExampleDataStorage* store)
{
        os << store->size() << " ";
        for(unsigned i = 0; i < store->size(); i++) {
		os << store->addresses[i] << " ";
        }
        os << std::endl;
        return os;
}

std::ifstream& operator>>(std::ifstream& is, ExampleDataStorage* store)
{
        unsigned size;
        IpAddress addr;
        is >> size;
        if(!is) 
		return is;
	store->addresses.clear();
	for(unsigned i = 0; i < size; i++) {
		is >> addr;
		store->addresses.push_back(addr);
	}
	return is;
}
