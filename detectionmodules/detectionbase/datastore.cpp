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

#include "datastore.h"


#include <ostream>
#include <sstream>


int DataStore::fieldToInt(byte* data, int len) 
{
        switch (len) {
        case 1:
                return *(uint8_t*)data;
        case 2:
                return  ntohs(*(uint16_t*)data);
        case 4:
                return ntohl(*(uint32_t*)data);
        case 8:
                return ntohll(*(uint64_t*)data);
        default:
                return -1;
        }
}

std::string IpAddress::toString() const
{
        std::stringstream sstream;
        sstream << address[0] << "." << address[1] << "." << address[2] << "." << address[3];
        return sstream.str();
}


std::ostream& operator<<(std::ostream& ost, const IpAddress& ip) 
{
        ost << ip[0] << "." << ip[1] << "." << ip[2] << "." << ip[3];
        return ost;
}
