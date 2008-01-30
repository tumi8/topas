/**************************************************************************/
/*    Copyright (C) 2006-07                                               */
/*    Romain Michalec, Sven Wiebusch                                      */
/*    University of Tuebingen, Germany                                    */
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
/*    License along with this library; if not, write to the Free Software   */
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,          */
/*    MA  02110-1301, USA                                                 */
/*                                                                        */
/**************************************************************************/

#include "shared.h"
#include <ostream>
#include <sstream>

// ============== SOME METHODS FOR CLASS EndPoint ================

// Needed to create filenames etc.
std::string EndPoint::toString() const
{
        std::stringstream sstream;
        sstream << ipAddr << "_" << portNr << "|" << (uint16_t) protocolID;
        return sstream.str();
}



// Creates an EndPoint from its string representation
// withNetMask = true --> string contains netmask
// withNetMask = false --> string doesnt contain netmask
void EndPoint::fromString(const std::string & epstr) {

  std::string::size_type i = epstr.find(':', 0);
  std::string ipstr(epstr, 0, i);
  std::string::size_type j = epstr.find('|', i);
  std::string portstr(epstr, i+1, j-i-1);
  std::string::size_type k = epstr.find('_', j);
  std::string protostr(epstr, j+1, k-j-1);

  ipAddr.fromString(ipstr);
  portNr = atoi(portstr.c_str());
  protocolID = atoi(protostr.c_str());

  return;
}

std::ostream& operator << (std::ostream& ost, const EndPoint& e)
{
        ost << e.ipAddr <<  ":" << e.portNr << "|" << (uint16_t) e.protocolID;
        return ost;
}

// ============== SOME METHODS FOR CLASS FilterEndPoint ================

void FilterEndPoint::fromString(const std::string & fepstr, bool withNetmask) {

  if (withNetmask == true) {
    std::string::size_type h = fepstr.find('/', 0);
    std::string ipstr(fepstr, 0, h);
    std::string::size_type i = fepstr.find(':', h);
    std::string netmaskstr(fepstr, h+1, i-h-1);
    std::string::size_type j = fepstr.find('|', i);
    std::string portstr(fepstr, i+1, j-i-1);
    std::string::size_type k = fepstr.find('_', j);
    std::string protostr(fepstr, j+1, k-j-1);

    ipAddr.fromString(ipstr);
    if (atoi(netmaskstr.c_str()) > 0 && atoi(netmaskstr.c_str()) < 32) {
      nmask = atoi(netmaskstr.c_str());
      ipAddr.remanent_mask(nmask);
    }
    else if (atoi(netmaskstr.c_str()) == 0) {
      nmask = 0;
    }
    else if (atoi(netmaskstr.c_str()) < 0 || atoi(netmaskstr.c_str()) > 32) {
      std::cerr << "WARNING:: Invalid Netmask occured in FilterEndPoint::fromString()!\n"
      << "  Netmask may only be a value "
      << "between 0 and 32! Thus, 32 will be assumed for now!\n";
      nmask = 32;
    }
    portNr = atoi(portstr.c_str());
    protocolID = atoi(protostr.c_str());
  }
  else {
    this->EndPoint::fromString(fepstr);
    nmask = 32;
  }

  return;
}

// Compares EndPoint ep with itself amd returns true, if they match
// and false otherwise
bool FilterEndPoint::matchesWithEndPoint (const EndPoint & ep, const short & netmask) {
  EndPoint tmp = ep;
  // apply the local netmask of the FilterEndPoint only,
  // if it is smaller than the global netmask
  // (otherwise, nothing would be changed)
  if ( nmask < netmask && nmask > 0 && nmask < 32)
    tmp.applyNetmask(nmask);
  // compare the members and watch out for wildcards
  if ( (tmp.getIpAddress() == ipAddr || nmask == 0)
    && (tmp.getPortNr() == portNr || portNr == -1)
    && (tmp.getProtocolID() == protocolID || protocolID == -1) )
    return true;

  return false;
}

std::ostream& operator << (std::ostream& ost, const FilterEndPoint& fep)
{
        ost << fep.getIpAddress() << "/" << fep.getNetmask() <<  ":" << fep.getPortNr() << "|" << (uint16_t) fep.getProtocolID();
        return ost;
}

// ======================== Output Operators ========================

std::ostream & operator << (std::ostream & os, const std::map<EndPoint,Info> & m) {
  std::map<EndPoint,Info>::const_iterator it = m.begin();
  while (it != m.end()){
    os << it->first << "_" << it->second.packets_in << " " << it->second.packets_out << " " << it->second.bytes_in << " " << it->second.bytes_out << " " << it->second.records_in << " " << it->second.records_out << "\n";
    it++;
  }
  return os;
}

std::ostream & operator << (std::ostream & os, const std::list<int64_t> & L) {
  std::list<int64_t>::const_iterator it = L.begin();
  while (it != L.end()) {
    os << *it << ',';
    it++;
  }
  return os;
}

std::ostream & operator << (std::ostream & os, const std::vector<unsigned> & V) {
  std::vector<unsigned>::const_iterator it = V.begin();
  while (it != V.end()) {
    os << *it << ',';
    it++;
  }
  return os;
}

std::ostream & operator << (std::ostream & os, const std::vector<int64_t> & V) {
  std::vector<int64_t>::const_iterator it = V.begin();
  os << "( ";
  while (it != V.end()) {
    os << *it << " ";
    it++;
  }
  os << ")";
  return os;
}

std::ostream & operator << (std::ostream & os, const std::vector<double> & V) {
std::vector<double>::const_iterator it = V.begin();
  os << "( ";
  while (it != V.end()) {
    os << *it << " ";
    it++;
  }
  os << ")";
  return os;
}

std::ostream & operator << (std::ostream & os, const std::list<std::vector<int64_t> > & L) {
  // We need a ckeck as L for sample_new is initially empty
  if (L.size() > 0) {
    std::list<std::vector<int64_t> >::const_iterator it = L.begin();
    os << *it; it++;
    while (it != L.end()) {
      os << ", " << *it;
      it++;
    }
  }
  return os;
}
