/**************************************************************************/
/*    Copyright (C) 2007 Gerhard Muenz                                    */
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

#ifndef _COUNTSTORE_H_
#define _COUNTSTORE_H_

#include <map>
#include <vector>
#include <ostream>
#include <stdexcept>
#include <datastore.h>
#include <iostream>
#include <concentrator/msg.h>
#include "bloomfilter.h"

template<unsigned size> class GenericKey
{
    public:
	unsigned len;
	uint8_t data[size];

	GenericKey() : len(0) {
	    memset(data,0,sizeof(data));
	}

	void set(uint8_t *input, unsigned inputlen)
	{
	    len = inputlen<sizeof(data)?inputlen:sizeof(data);
	    memcpy(&data, input, len);
	}

	void reset()
	{
	    len = 0;
	    memset(data,0,sizeof(data));
	}
	
	void append(uint8_t *input, unsigned inputlen)
	{
	    if(len<sizeof(data)) {
		if((len+inputlen) < sizeof(data)) {
		    memcpy(&(data[len]), input, inputlen);
		    len = len + inputlen;
		} else {
		    memcpy(&(data[len]), input, sizeof(data)-len);
		    len = sizeof(data);
		}
	    }
	}
	
	bool operator<(const GenericKey& other) const 
	{
	    if(len < other.len)
		return true;
	    else if(len > other.len)
		return false;
	    else
		return memcmp(data, other.data, len)<0?true:false;
	}

};


struct Counters {
    public:
	Counters(): octetCount(0), packetCount(0), flowCount(0) {}
	Counters(uint64_t octets, uint64_t packets, uint64_t flows)
	    : octetCount(octets), packetCount(packets), flowCount(flows) {}
	~Counters() {}

	void update(uint64_t octets, uint64_t packets, uint64_t flows)
	{
	    octetCount += octets;
	    packetCount += packets;
	    flowCount += flows;
	}
	
	uint64_t octetCount, packetCount, flowCount;
};


class CountStore : public DataStore 
{
    public:
	typedef GenericKey<15> FiveTuple;
	typedef std::map<IpAddress, Counters> IpCountMap;
	typedef std::map<uint32_t, Counters> PortCountMap;
	typedef uint32_t ProtoPort;

	CountStore() : recordStarted(false)
	{
	    bfilter.clear();
	}

	~CountStore() {}

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

	static void init(uint32_t size, unsigned hashfunctions)
	{
	    bfilter.init(size, hashfunctions);
	}

	static bool countPerSrcIp, countPerDstIp, countPerSrcPort, countPerDstPort;
	    
	IpCountMap srcIpCounts, dstIpCounts;
	PortCountMap srcPortCounts, dstPortCounts;

    private:
	void updateIpCountMap(IpCountMap& countmap, IpCountMap::iterator& iter, const IpAddress& addr, const bool newFlowKey);
	void updatePortCountMap(PortCountMap& countmap, PortCountMap::iterator& iter, ProtoPort port, const bool newFlowKey);
	
	static BloomFilter bfilter;
	
	IpAddress srcIp, dstIp;
	ProtoPort  srcPort, dstPort;
	uint64_t octets, packets;

	FiveTuple flowKey;

	bool recordStarted;
};

#endif
