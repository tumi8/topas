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

#include "ipaddress.h"

#include <ostream>
#include <sstream>


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


std::istream& operator>>(std::istream& ist, IpAddress& ip) 
{
	unsigned i[4];
	char dot;
        if(ist >> i[0] >> dot >> i[1] >> dot >> i[2] >> dot >> i[3])
		ip.setAddress(i[0], i[1], i[2], i[3]);
        return ist;
}
