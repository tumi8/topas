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

#ifndef _SHARED_H_
#define _SHARED_H_

#include <ipaddress.h>
#include <list>
#include <map>
#include <vector>
#include <iostream>

// ======================== STRUCT Info ========================

// this structure is used to store traffic information
// about a node

struct Info {
  uint64_t packets_in;
  uint64_t packets_out;
  uint64_t bytes_in;
  uint64_t bytes_out;
  uint64_t records_in;
  uint64_t records_out;
};


// ==================== ENDPOINT CLASS EndPoint ====================
class EndPoint {

  protected:

    IpAddress ipAddr;
    int portNr;
    int protocolID;

    friend std::ostream& operator << (std::ostream&, const EndPoint&);

  public:

    // Constructors
    EndPoint() : ipAddr (0,0,0,0), portNr(0), protocolID(0) {}

    EndPoint(const IpAddress & ip, int port, int protocol) : ipAddr (ip[0], ip[1], ip[2], ip[3]), portNr(port), protocolID(protocol) {}

    // copy constructor
    EndPoint(const EndPoint & e) : ipAddr(e.ipAddr[0], e.ipAddr[1], e.ipAddr[2], e.ipAddr[3]){

      portNr = e.portNr;
      protocolID = e.protocolID;

    }

    // Destructor
    ~EndPoint() {};

    // for the following two operators, netmask doesnt need to be compared,
    // because it was already applied to the ip address

    // Operators (needed for use in maps)
    bool operator==(const EndPoint& e) const {
      return ipAddr[0] == e.ipAddr[0] && ipAddr[1] == e.ipAddr[1]
                && ipAddr[2] == e.ipAddr[2] && ipAddr[3] == e.ipAddr[3]
              && portNr == e.portNr && protocolID == e.protocolID;
    }

    bool operator<(const EndPoint& e) const {
      if (ipAddr[0] < e.ipAddr[0])
        return true;
      if (ipAddr[0] == e.ipAddr[0] && ipAddr[1] < e.ipAddr[1])
        return true;
      if (ipAddr[0] == e.ipAddr[0] && ipAddr[1] == e.ipAddr[1]
    && ipAddr[2] < e.ipAddr[2])
        return true;
      if (ipAddr[0] == e.ipAddr[0] && ipAddr[1] == e.ipAddr[1]
    && ipAddr[2] == e.ipAddr[2] && ipAddr[3] < e.ipAddr[3])
        return true;
      if (ipAddr[0] == e.ipAddr[0] && ipAddr[1] == e.ipAddr[1]
    && ipAddr[2] == e.ipAddr[2] && ipAddr[3] == e.ipAddr[3] && portNr < e.portNr)
        return true;
      if (ipAddr[0] == e.ipAddr[0] && ipAddr[1] == e.ipAddr[1]
    && ipAddr[2] == e.ipAddr[2] && ipAddr[3] == e.ipAddr[3] && portNr == e.portNr && protocolID < e.protocolID)
        return true;

      return false;
    }

    std::string toString() const;
    void fromString(const std::string &);

    // Setters & Getters
    void setIpAddress(const IpAddress & ip) {
      ipAddr.setAddress(ip[0], ip[1], ip[2], ip[3]);
    }

    void setPortNr(const int & p) {
      portNr = p;
    }

    void setProtocolID(const int & pid) {
      protocolID = pid;
    }

    IpAddress getIpAddress() const { return ipAddr; }
    int getPortNr() const { return portNr; }
    int getProtocolID() const { return protocolID; }

    void applyNetmask(short n) {
      ipAddr.remanent_mask(n);
    }

};

// ==================== FILTER CLASS FilterEndPoint ====================

class FilterEndPoint : public EndPoint
{

  private:

    short nmask;

    friend std::ostream& operator << (std::ostream&, const FilterEndPoint&);

  public:

    FilterEndPoint() {
      nmask = 32;
    }

    ~FilterEndPoint() {};

    // this does the same as the fromString() method of
    // the base class, but can additionally handle netmask
    void fromString(const std::string &, bool);

    // Tests, if the endpoint given by the parameter matches with
    // the FilterEndPoint, after the netmask was applied
    bool matchesWithEndPoint (const EndPoint &, const short &);

    void setNetmask (short n) {
      if (n >= 0 && n <= 32)
        nmask = n;
      else {
        std::cerr << "WARNING: Invalid netmask supplied for FilterEndPoint::setNetmask()!\n"
        << "  \"32\" assumed instead.\n";
        nmask = 32;
      }
    }
    short getNetmask () const { return nmask; }

};


// ======================== Output Operators ========================

std::ostream & operator << (std::ostream &, const std::list<int64_t> &);
std::ostream & operator << (std::ostream &, const std::vector<unsigned> &);
std::ostream & operator << (std::ostream &, const std::vector<int64_t> &);
std::ostream & operator << (std::ostream &, const std::vector<double> &);
std::ostream & operator << (std::ostream &, const std::list<std::vector<int64_t> > &);
std::ostream & operator << (std::ostream &, const std::map<EndPoint,Info> &);

#endif
