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

#include "stat-store.h"
#include <fstream>

#include <string.h>


// ==================== STORAGE CLASS StatStore ====================


StatStore::StatStore()
    : e_source(IpAddress(0,0,0,0),0,0), e_dest(IpAddress(0,0,0,0),0,0) {

	packet_nb = byte_nb = 0;

    }

StatStore::~StatStore() {

    PreviousData = Data;

}

bool StatStore::recordStart(SourceID sourceId) {

    if (beginMonitoring != true)
	return false;

    packet_nb = byte_nb = 0;
    e_source = e_dest = EndPoint(IpAddress(0,0,0,0),0,0);

    return true;

}

void StatStore::addFieldData(int id, byte * fieldData, int fieldDataLength, EnterpriseNo eid) {
    // we subscribed to: see Stat::init_*()-functions
    // addFieldData will be executed until there are no more fieldData
    // in the current IPFIX record; so we are sure to get everything
    // we subscribed to (so don't get worried because of
    // the "breaks" in the "switch" loop hereafter)

    IpAddress SourceIP = IpAddress(0,0,0,0);
    IpAddress DestIP = IpAddress(0,0,0,0);

    switch (id) {

	case IPFIX_TYPEID_protocolIdentifier:

	    if (fieldDataLength != IPFIX_LENGTH_protocolIdentifier) {
		msgStr << MsgStream::ERROR << "Got IPFIX field (id=" << id << ") with unsupported length " << fieldDataLength << ". Skipping record." << MsgStream::endl;
		return;
	    }

	    e_source.setProtocolID(*fieldData);
	    e_dest.setProtocolID(*fieldData);

	    break;


	case IPFIX_TYPEID_sourceIPv4Address:

	    if (fieldDataLength != IPFIX_LENGTH_sourceIPv4Address) {
		msgStr << MsgStream::ERROR << "Got IPFIX field (id=" << id << ") with unsupported length " << fieldDataLength << ". Skipping record." << MsgStream::endl;
		return;
	    }

	    SourceIP.setAddress(fieldData[0],fieldData[1],fieldData[2],fieldData[3]);
	    SourceIP.remanent_mask(netmask);
	    e_source.setIpAddress(SourceIP);

	    break;


	case IPFIX_TYPEID_destinationIPv4Address:

	    if (fieldDataLength != IPFIX_LENGTH_destinationIPv4Address) {
		msgStr << MsgStream::ERROR << "Got IPFIX field (id=" << id << ") with unsupported length " << fieldDataLength << ". Skipping record." << MsgStream::endl;
		return;
	    }

	    DestIP.setAddress(fieldData[0],fieldData[1],fieldData[2],fieldData[3]);
	    DestIP.remanent_mask(netmask);
	    e_dest.setIpAddress(DestIP);

	    break;

	    // Ports do only matter, if endpoint_key contains "port"
	    // AND (endpoint_key contains "protocol" AND TCP and/or UDP are selected
	    // OR protocols dont matter)
	case IPFIX_TYPEID_sourceTransportPort:

	    if (fieldDataLength != IPFIX_LENGTH_sourceTransportPort
		    && fieldDataLength != IPFIX_LENGTH_sourceTransportPort-1) {
		msgStr << MsgStream::ERROR << "Got IPFIX field (id=" << id << ") with unsupported length " << fieldDataLength << ". Skipping record." << MsgStream::endl;
		return;
	    }

	    e_source.setPortNr((int)fieldToInt(fieldData, fieldDataLength));
	    break;


	case IPFIX_TYPEID_destinationTransportPort:

	    if (fieldDataLength != IPFIX_LENGTH_destinationTransportPort
		    && fieldDataLength != IPFIX_LENGTH_destinationTransportPort-1) {
		std::cerr << "Error! Got invalid IPFIX field data (destination port)! "
		    << "Skipping record.\n";
		return;
	    }

	    e_dest.setPortNr((int)fieldToInt(fieldData, fieldDataLength));
	    break;


	case IPFIX_TYPEID_packetDeltaCount:

	    if (fieldDataLength != IPFIX_LENGTH_packetDeltaCount) {
		std::cerr << "Error! Got invalid IPFIX field data (#packets)! "
		    << "Skipping record.\n";
		return;
	    }
	    packet_nb = ntohll(*(uint64_t*)fieldData);

	    break;


	case IPFIX_TYPEID_octetDeltaCount:

	    if (fieldDataLength != IPFIX_LENGTH_octetDeltaCount) {
		std::cerr << "Error! Got invalid IPFIX field data (#octets)! "
		    << "Skipping record.\n";
		return;
	    }
	    byte_nb = ntohll(*(uint64_t*)fieldData);

	    break;


	default:

	    std::cerr
		<<"Warning! Got unknown record in StatStore::addFieldData(...)!\n"
		<<"A programmer has probably added some record type in Stat::init()\n"
		<<"but has forgotten to ensure its support in StatStore::addFieldData().\n"
		<<"I'll try to keep working, but I can't tell for sure I won't crash.\n";
    }

    return;
}


// For every call to recordEnd, two endpoints will be considered:
// One consisting of SourceIP, SourcePort and Protocol (e_source)
// And one consisting of DestIP, DestPort and Protocol (e_dest)
void StatStore::recordEnd() {

    std::stringstream Warning;
    Warning
	<< "WARNING: New EndPoint observed but EndPointListMaxSize reached!\n"
	<< "Couldn't monitor new EndPoint: ";

    // Handle EndPoint e_source (with SourceIP and SourcePort)

    // FILTER: Consider only EndPoints we are interested in
    if (monitorEndPoint(e_source) == true || monitorEveryEndPoint == true) {

	// EndPoint already known and thus in our List?
	if ( find(endPointList.begin(), endPointList.end(), e_source) != endPointList.end() ) {
	    // Since Data is destroyed after every test()-run,
	    // we need to check, if the endpoint was already seen in the
	    // current run
	    std::map<EndPoint,Info>::iterator it = Data.find(e_source);
	    if( it != Data.end() ) {
		it->second.packets_out += packet_nb;
		it->second.bytes_out += byte_nb;
		it->second.records_out++;
	    }
	    else {
		Data[e_source].packets_out += packet_nb;
		it = Data.find(e_source);
		it->second.bytes_out += byte_nb;
		it->second.records_out++;
	    }
	}
	// EndPoint not known, still place to add it?
	else {
	    if (endPointList.size() < endPointListMaxSize) {
		Info newinfo;
		newinfo.bytes_in = newinfo.packets_in = newinfo.records_in = 0;
		newinfo.packets_out = packet_nb;
		newinfo.bytes_out = byte_nb;
		newinfo.records_out = 1;
		Data.insert(std::make_pair<EndPoint,Info>(e_source, newinfo));
		endPointList.push_back(e_source);
	    }
	    else
		std::cerr << Warning.str() << e_source << std::endl;
	}
    }


    // Handle EndPoint e_dest (with DestIP and DestPort)

    // FILTER: Consider only EndPoints we are interested in
    if (monitorEndPoint(e_dest) == true || monitorEveryEndPoint == true) {

	// EndPoint already known and thus in our List?
	if ( find(endPointList.begin(), endPointList.end(), e_dest) != endPointList.end() ) {
	    // Since Data is destroyed after every test()-run,
	    // we need to check, if the endpoint was already seen in the
	    // current run
	    std::map<EndPoint,Info>::iterator it = Data.find(e_dest);
	    if ( it != Data.end() ) {
		it->second.packets_in += packet_nb;
		it->second.bytes_in += byte_nb;
		it->second.records_in++;
	    }
	    else {
		Data[e_dest].packets_in += packet_nb;
		it = Data.find(e_dest);
		it->second.bytes_in += byte_nb;
		it->second.records_in++;
	    }
	}
	// EndPoint not known, still place to add it?
	else {
	    if (endPointList.size() < endPointListMaxSize) {
		Info newinfo;
		newinfo.bytes_out = newinfo.packets_out = newinfo.records_out = 0;
		newinfo.packets_in = packet_nb;
		newinfo.bytes_in = byte_nb;
		newinfo.records_in = 1;
		Data.insert(std::make_pair<EndPoint,Info>(e_dest, newinfo));
		endPointList.push_back(e_dest);
	    }
	    else
		std::cerr << Warning.str() << e_dest << std::endl;
	}
    }

    return;
}

// input from file (for offline usage)
std::ifstream& operator>>(std::ifstream& is, StatStore* store) {

    std::stringstream Warning;
    Warning
	<< "WARNING: New EndPoint observed but endPointListMaxSize reached!\n"
	<< "Couldn't monitor new EndPoint: ";

    if ( is.eof() ) {
	std::cerr << "INFORMATION: All Data read from file.\n";
	is.close();
	return is;
    }

    std::string tmp;
    store->Data.clear();
    while ( getline(is, tmp) ) {
	if (0 == strncmp("---",tmp.c_str(),3) )
	    break;
	else if ( is.eof() ) {
	    std::cerr << "INFORMATION: All Data read from file.\n";
	    is.close();
	    return is;
	}

	// extract endpoint-data
	EndPoint ep;
	ep.fromString(tmp);
	// extract metric-data
	std::string::size_type k = tmp.find('_', 0);
	std::stringstream tmp1(tmp.substr(k+1));
	Info info;
	tmp1 >> info.packets_in >> info.packets_out >> info.bytes_in >> info.bytes_out >> info.records_in >> info.records_out;

	// AGGREGATION: Use endpoint_key and netmask parameters to aggregate endpoints
	if (store->ipMonitoring == false)
	    ep.setIpAddress(IpAddress(0,0,0,0));
	else // apply global netmask
	    ep.applyNetmask(store->netmask);
	if (store->portMonitoring == false)
	    ep.setPortNr(0);
	if (store->protocolMonitoring == false)
	    ep.setProtocolID(0);

	// FILTER: Consider only EndPoints we are interested in
	if (store->monitorEndPoint(ep) == true || store->monitorEveryEndPoint == true) {
	    // EndPoint already known and thus in our List?
	    if ( find(store->endPointList.begin(), store->endPointList.end(), ep) != store->endPointList.end() ) {
		// Since Data is destroyed after every test()-run,
		// we need to check, if the endpoint was already seen in the
		// current run
		std::map<EndPoint,Info>::iterator it = store->Data.find(ep);
		if ( it != store->Data.end() ) {
		    it->second.packets_in += info.packets_in;
		    it->second.bytes_in += info.bytes_in;
		    it->second.records_in += info.records_in;
		    it->second.packets_out += info.packets_out;
		    it->second.bytes_out += info.bytes_out;
		    it->second.records_out += info.records_out;
		}
		else {
		    store->Data[ep].packets_in += info.packets_in;
		    it = store->Data.find(ep);
		    it->second.bytes_in += info.bytes_in;
		    it->second.records_in += info.records_in;
		    it->second.packets_out += info.packets_out;
		    it->second.bytes_out += info.bytes_out;
		    it->second.records_out += info.records_out;
		}
	    }
	    // EndPoint not known, still place to add it?
	    else {
		if (store->endPointList.size() < store->endPointListMaxSize) {
		    store->Data.insert(std::make_pair<EndPoint,Info>(ep, info));
		    store->endPointList.push_back(ep);
		}
		else
		    std::cerr << Warning.str() << ep << std::endl;
	    }
	}

	tmp.clear();
	tmp1.clear();
    }

    return is;
}

// returns true, if we are interested in EndPoint ep.
// That means, that ep matches one of the FilterEndPoints defined
// in endPointFilter (initialized either by x_frequently_endpoints
// or endpoints_to_monitor file
// (returns false otherwise)
bool StatStore::monitorEndPoint (const EndPoint & ep) {

    std::vector<FilterEndPoint>::iterator it = endPointFilter.begin();
    while ( it != endPointFilter.end() ) {
	if (it->matchesWithEndPoint(ep, netmask) == true)
	    return true;
	//    std::cout << *it << "\t | \t" << ep << std::endl;
	it++;
    }

    return false;
}


std::string StatStore::EndPointFilters()
{
    std::stringstream tmp;
    for(std::vector<FilterEndPoint>::iterator it = endPointFilter.begin();
	    it != endPointFilter.end(); it++ ) {
	tmp << *it << "\n"; 
    }
    return tmp.str();
}

// ========== INITIALISATIONS OF STATIC MEMBERS OF CLASS StatStore ===========

std::map<EndPoint,Info> StatStore::PreviousData;

// even if the following members will be given their actual values
// by the Stat::init() function, we have to provide some initial values
// in the implementation file of the related class;

short StatStore::netmask = 32;
// for OFFLINE MODE
bool StatStore::ipMonitoring = false;
bool StatStore::portMonitoring = false;
bool StatStore::protocolMonitoring = false;

std::vector<FilterEndPoint> StatStore::endPointFilter;
bool StatStore::monitorEveryEndPoint = false;

std::vector<EndPoint> StatStore::endPointList;
int StatStore::endPointListMaxSize = 0;

bool StatStore::beginMonitoring = false;
