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

void IpAddress::fromString(const std::string & ipstr)
{
        std::string::size_type i_alt = 0;
        std::string::size_type i_neu = ipstr.find('.',i_alt);
        int index = 0;
        while ( true ) {
          address[index] = atoi( (ipstr.substr(i_alt,i_neu-i_alt)).c_str() );
          if ( i_neu == ipstr.length())
            break;
          i_alt = i_neu+1;
          i_neu = ipstr.find('.',i_alt);
          if ( i_neu == std::string::npos)
            i_neu = ipstr.length();
          index++;
        }
        return;
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

IpAddress IpAddress::mask (short nmask) {
  unsigned int mask[4];
  std::string Mask;
  switch( nmask ) {
    case  0: Mask="0x00000000"; break;
    case  1: Mask="0x80000000"; break;
    case  2: Mask="0xC0000000"; break;
    case  3: Mask="0xE0000000"; break;
    case  4: Mask="0xF0000000"; break;
    case  5: Mask="0xF8000000"; break;
    case  6: Mask="0xFC000000"; break;
    case  7: Mask="0xFE000000"; break;
    case  8: Mask="0xFF000000"; break;
    case  9: Mask="0xFF800000"; break;
    case 10: Mask="0xFFC00000"; break;
    case 11: Mask="0xFFE00000"; break;
    case 12: Mask="0xFFF00000"; break;
    case 13: Mask="0xFFF80000"; break;
    case 14: Mask="0xFFFC0000"; break;
    case 15: Mask="0xFFFE0000"; break;
    case 16: Mask="0xFFFF0000"; break;
    case 17: Mask="0xFFFF8000"; break;
    case 18: Mask="0xFFFFC000"; break;
    case 19: Mask="0xFFFFE000"; break;
    case 20: Mask="0xFFFFF000"; break;
    case 21: Mask="0xFFFFF800"; break;
    case 22: Mask="0xFFFFFC00"; break;
    case 23: Mask="0xFFFFFE00"; break;
    case 24: Mask="0xFFFFFF00"; break;
    case 25: Mask="0xFFFFFF80"; break;
    case 26: Mask="0xFFFFFFC0"; break;
    case 27: Mask="0xFFFFFFE0"; break;
    case 28: Mask="0xFFFFFFF0"; break;
    case 29: Mask="0xFFFFFFF8"; break;
    case 30: Mask="0xFFFFFFFC"; break;
    case 31: Mask="0xFFFFFFFE"; break;
    case 32: Mask="0xFFFFFFFF"; break;
  }
  char mask1[3] = {Mask[2],Mask[3],'\0'};
  char mask2[3] = {Mask[4],Mask[5],'\0'};
  char mask3[3] = {Mask[6],Mask[7],'\0'};
  char mask4[3] = {Mask[8],Mask[9],'\0'};
  mask[0] = strtol (mask1, NULL, 16);
  mask[1] = strtol (mask2, NULL, 16);
  mask[2] = strtol (mask3, NULL, 16);
  mask[3] = strtol (mask4, NULL, 16);

  return IpAddress( address[0] & mask[0], address[1] & mask[1], address[2] & mask[2], address[3] & mask[3] );
}

void IpAddress::remanent_mask (short nmask) {
  unsigned int mask[4];
  std::string Mask;
  switch( nmask ) {
    case  0: Mask="0x00000000"; break;
    case  1: Mask="0x80000000"; break;
    case  2: Mask="0xC0000000"; break;
    case  3: Mask="0xE0000000"; break;
    case  4: Mask="0xF0000000"; break;
    case  5: Mask="0xF8000000"; break;
    case  6: Mask="0xFC000000"; break;
    case  7: Mask="0xFE000000"; break;
    case  8: Mask="0xFF000000"; break;
    case  9: Mask="0xFF800000"; break;
    case 10: Mask="0xFFC00000"; break;
    case 11: Mask="0xFFE00000"; break;
    case 12: Mask="0xFFF00000"; break;
    case 13: Mask="0xFFF80000"; break;
    case 14: Mask="0xFFFC0000"; break;
    case 15: Mask="0xFFFE0000"; break;
    case 16: Mask="0xFFFF0000"; break;
    case 17: Mask="0xFFFF8000"; break;
    case 18: Mask="0xFFFFC000"; break;
    case 19: Mask="0xFFFFE000"; break;
    case 20: Mask="0xFFFFF000"; break;
    case 21: Mask="0xFFFFF800"; break;
    case 22: Mask="0xFFFFFC00"; break;
    case 23: Mask="0xFFFFFE00"; break;
    case 24: Mask="0xFFFFFF00"; break;
    case 25: Mask="0xFFFFFF80"; break;
    case 26: Mask="0xFFFFFFC0"; break;
    case 27: Mask="0xFFFFFFE0"; break;
    case 28: Mask="0xFFFFFFF0"; break;
    case 29: Mask="0xFFFFFFF8"; break;
    case 30: Mask="0xFFFFFFFC"; break;
    case 31: Mask="0xFFFFFFFE"; break;
    case 32: Mask="0xFFFFFFFF"; break;
  }
  char mask1[3] = {Mask[2],Mask[3],'\0'};
  char mask2[3] = {Mask[4],Mask[5],'\0'};
  char mask3[3] = {Mask[6],Mask[7],'\0'};
  char mask4[3] = {Mask[8],Mask[9],'\0'};
  mask[0] = strtol (mask1, NULL, 16);
  mask[1] = strtol (mask2, NULL, 16);
  mask[2] = strtol (mask3, NULL, 16);
  mask[3] = strtol (mask4, NULL, 16);

  setAddress( address[0] & mask[0], address[1] & mask[1], address[2] & mask[2], address[3] & mask[3] );

  return;
}
