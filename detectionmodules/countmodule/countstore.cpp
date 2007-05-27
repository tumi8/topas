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

#include "countstore.h"
#include "countmodule.h"

#include <cassert>


BloomFilter CountStore::bfilter;
bool CountStore::countPerSrcIp = false;
bool CountStore::countPerDstIp = false;
bool CountStore::countPerSrcPort = false;
bool CountStore::countPerDstPort = false;


void CountStore::addFieldData(int id, byte* fieldData, int fieldDataLength, EnterpriseNo eid) 
{
    switch (id) {
	case IPFIX_TYPEID_sourceIPv4Address:
	    /* fieldDataLength == 5 means fieldData == xxx.yyy.zzz/aa (ip address and netmask)
	       fieldDataLength == 4 means fieldData == xxx.yyy.zzz/0  (ip address)
	       */
	    if(fieldDataLength>=4)
	    {
		srcIp.setAddress(fieldData);
		flowKey.append(fieldData, 4); // we ignore the netmask
	    }
	    else
		msgStr.print(MsgStream::ERROR, "IP address field too short.");
	    break;
	case IPFIX_TYPEID_destinationIPv4Address:
	    /* fieldDataLength == 5 means fieldData == xxx.yyy.zzz/aa (ip address and netmask)
	       fieldDataLength == 4 means fieldData == xxx.yyy.zzz/0  (ip address)
	       */
	    if(fieldDataLength>=4)
	    {
		dstIp.setAddress(fieldData);
		flowKey.append(fieldData, 4); // we ignore the netmask
	    }
	    else
		msgStr.print(MsgStream::ERROR, "IP address field too short.");
	    break;
	case IPFIX_TYPEID_sourceTransportPort:
	    if (fieldDataLength == 2)
	    {
		srcPort = (srcPort & 0xFFFF0000) + ntohs(*(uint16_t*)fieldData);
		flowKey.append(fieldData, 2);
	    }
	    else
		msgStr.print(MsgStream::ERROR, "Invalid port field length.");
	    break;
	case IPFIX_TYPEID_destinationTransportPort:
	    if (fieldDataLength == 2)
	    {
		dstPort = (dstPort & 0xFFFF0000) + ntohs(*(uint16_t*)fieldData);
		flowKey.append(fieldData, 2);
	    }
	    else
		msgStr.print(MsgStream::ERROR, "Invalid port field length.");
	    break;
	case IPFIX_TYPEID_protocolIdentifier:
	    if (fieldDataLength == 1)
	    {
		srcPort = (srcPort & 0x0000FFFF) + ((*fieldData)<<16);
		dstPort = (dstPort & 0x0000FFFF) + ((*fieldData)<<16);
		flowKey.append(fieldData, 1);
	    }
	    else
		msgStr.print(MsgStream::ERROR, "Invalid protocol field length.");
	    break;
	case IPFIX_TYPEID_octetDeltaCount:
	    octets = fieldToInt(fieldData, fieldDataLength);
	    break;
	case IPFIX_TYPEID_packetDeltaCount:
	    packets = fieldToInt(fieldData, fieldDataLength);
	    break;
	default:
	    break;
    }
}


bool CountStore::recordStart(SourceID id) 
{
    assert(recordStarted == false);
    flowKey.reset();
    recordStarted = true;

    srcIp.setAddress(0,0,0,0);
    dstIp.setAddress(0,0,0,0);
    srcPort = dstPort = 0;
    packets = octets = 0;

    return true;
}

void CountStore::recordEnd() 
{
    assert(recordStarted == true);

    bool newFlowKeyBf = (bfilter.testBeforeInsert(flowKey.data,flowKey.len) == false);
    bool newFlowKey = newFlowKeyBf;

    IpCountMap::iterator srcIpIter, dstIpIter; 
    PortCountMap::iterator srcPortIter, dstPortIter; 
    
    // search flow key in tables first to eventually detect bloom filter colisions
    if(countPerSrcIp)
    {	
	srcIpIter = srcIpCounts.find(srcIp);
	if(srcIpIter == srcIpCounts.end())
	    newFlowKey = true;
    }
    if(countPerDstIp)
    {	
	dstIpIter = dstIpCounts.find(dstIp);
	if(dstIpIter == dstIpCounts.end())
	    newFlowKey = true;
    }
    if(countPerSrcPort)
    {	
	srcPortIter = srcPortCounts.find(srcPort);
	if(srcPortIter == srcPortCounts.end())
	    newFlowKey = true;
    }
    if(countPerDstPort)
    {	
	dstPortIter = dstPortCounts.find(dstPort);
	if(dstPortIter == dstPortCounts.end())
	    newFlowKey = true;
    }

    if(newFlowKey)
	msgStr.print(MsgStream::INFO, "New record received: IP-5-tuple is new.");
    else
	msgStr.print(MsgStream::INFO, "New record received: IP-5-tuple is already known.");
	
    if(newFlowKey != newFlowKeyBf)
	msgStr.print(MsgStream::WARN, "There was a bloomfilter collision!");

    // now update the tables
    // SrcIp
    if(countPerSrcIp)
    {
	msgStr << MsgStream::INFO << "SrcIp: ";
	updateIpCountMap(srcIpCounts, srcIpIter, srcIp, newFlowKey);
    }

    // DstIp
    if(countPerDstIp)
    {
	msgStr << MsgStream::INFO << "DstIp: ";
	updateIpCountMap(dstIpCounts, dstIpIter, dstIp, newFlowKey);
    }

    // SrcPort
    if(countPerSrcPort)
    {
	msgStr << MsgStream::INFO << "SrcPort: ";
	updatePortCountMap(srcPortCounts, srcPortIter, srcPort, newFlowKey);
    }

    // DstPort
    if(countPerDstPort)
    {
	msgStr << MsgStream::INFO << "DstPort: ";
	updatePortCountMap(dstPortCounts, dstPortIter, dstPort, newFlowKey);
    }

    recordStarted = false;
}

void CountStore::updateIpCountMap(IpCountMap& countmap, IpCountMap::iterator& iter, const IpAddress& addr, const bool newFlowKey)
{
    if(iter != countmap.end()) 
    {
	if(newFlowKey) 
	{
	    msgStr << "Update octets, packets, and flows.";
	    iter->second.update(octets, packets, 1);
	}
	else
	{
	    msgStr << "Update octets and packets only.";
	    iter->second.update(octets, packets, 0);
	}
    }
    else if(countmap.size() < (countmap.max_size()-1))
    {
	msgStr << "Create new table entry.";
	countmap.insert(std::pair<IpAddress,Counters>(addr, Counters(octets, packets, 1)));
    }
    else
    {
	msgStr << "I'm out of memory. Update default table entry.";
	if((iter = countmap.find(IpAddress(0,0,0,0))) != countmap.end())
	{
	    if(newFlowKey)
		iter->second.update(octets, packets, 1);
	    else
		iter->second.update(octets, packets, 0);
	}
	else
	    countmap.insert(std::pair<IpAddress,Counters>(IpAddress(0,0,0,0), Counters(octets, packets, 1)));
    }

    if(CountModule::verbose)
	if((iter = countmap.find(addr)) != countmap.end())
	    msgStr << " Table entry: " << iter->first.toString().c_str() 
		<< " o:"<< iter->second.octetCount <<" p:" << iter->second.packetCount << " f:" << iter->second.flowCount
		<< MsgStream::endl;
}

void CountStore::updatePortCountMap(PortCountMap& countmap, PortCountMap::iterator& iter, ProtoPort port, const bool newFlowKey)
{
    if(iter != countmap.end()) 
    {
	if(newFlowKey) 
	{
	    msgStr << "Update octets, packets, and flows.";
	    iter->second.update(octets, packets, 1);
	}
	else
	{
	    msgStr << "Update octets and packets only.";
	    iter->second.update(octets, packets, 0);
	}
    }
    else if(countmap.size() < (countmap.max_size()-1))
    {
	msgStr << "Create new table entry.";
	countmap.insert(std::pair<ProtoPort,Counters>(port, Counters(octets, packets, 1)));
    }
    else
    {
	msgStr << "I'm out of memory. Update default table entry.";
	if((iter = countmap.find(0)) != countmap.end())
	{
	    if(newFlowKey)
		iter->second.update(octets, packets, 1);
	    else
		iter->second.update(octets, packets, 0);
	}
	else
	    countmap.insert(std::pair<ProtoPort,Counters>(0, Counters(octets, packets, 1)));
    }
    
    if(CountModule::verbose)
	if((iter = countmap.find(port)) != countmap.end())
	    msgStr << " Table entry: " << (iter->first >> 16) << "." << (iter->first & 0x0000FFFF)
		<< " o:"<< iter->second.octetCount <<" p:" << iter->second.packetCount << " f:" << iter->second.flowCount
		<< MsgStream::endl;
}
