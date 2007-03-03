/**************************************************************************/
/*    Copyright (C) 2006 Romain Michalec                                  */
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


// ==================== STORAGE CLASS StatStore ====================


StatStore::StatStore()
  : SourceIP (0,0,0,0), DestIP (0,0,0,0) {

  gotSourceIP = gotDestIP = false;
  gotProtocol = false;
  gotSourcePort = gotDestPort = false;

  packet_nb = byte_nb = 0;

  IpListMaxSizeReachedAndNewIpWantedToEnterIt = 0;

}

StatStore::~StatStore() {

  PreviousData = Data;

}

bool StatStore::recordStart(SourceID sourceId) {

  if (BeginMonitoring != true)
    return false;

  if (find(accept_source_ids->begin(),accept_source_ids->end(),(int)sourceId)==accept_source_ids->end()){
    return false;
  }

  gotSourceIP = gotDestIP = false;
  gotProtocol = false;
  gotSourcePort = gotDestPort = false;

  packet_nb = byte_nb = 0;

  SourceIP = DestIP = IpAddress(0,0,0,0);

  return true;

}

std::vector<int>* StatStore::accept_source_ids = NULL;
void StatStore::addFieldData(int id, byte * fieldData, int fieldDataLength, EnterpriseNo eid) {

  // we subscribed to (see Stat::init()):
  // - IPFIX_TYPEID_sourceIPv4Address and IPFIX_TYPEID_destinationIPv4Address
  // - IPFIX_TYPEID_protocolIdentifier
  // - IPFIX_TYPEID_packetDeltaCount and/or IPFIX_TYPEID_octetDeltaCount
  // - possibly, IPFIX_TYPEID_sourceTransportPort and
  //   IPFIX_TYPEID_destinationTransportPort

  // addFieldData will be executed until there are no more fieldData
  // in the current IPFIX record; so we are sure to get everything
  // we subscribed to (so don't get worried because of
  // the "breaks" in the "switch" loop hereafter)


  switch (id) {


  case IPFIX_TYPEID_protocolIdentifier:

    if (fieldDataLength != IPFIX_LENGTH_protocolIdentifier) {
      std::cerr << "Error! Got invalid IPFIX field data (protocol)! "
		<< "Skipping record.\n";
      return;
    }

    protocol = *fieldData; // useful in recordEnd()
    
    if ( find(MonitoredProtocols.begin(), MonitoredProtocols.end(), *fieldData)
	 !=
	 MonitoredProtocols.end() )
      gotProtocol = true;
    // *fielData is a protocol number, so is 1 byte (= uint8_t) long
    // remember to cast it into an uint16_t (= unsigned int)
    // if you want to print it!
    break;


  case IPFIX_TYPEID_sourceIPv4Address:

    if (fieldDataLength != IPFIX_LENGTH_sourceIPv4Address) {
      std::cerr << "Error! Got invalid IPFIX field data (source IP)! "
		<< "Skipping record.\n";
      return;
    }

    SourceIP = IpAddress(fieldData[0],fieldData[1],fieldData[2],fieldData[3]);

    if (MonitorEveryIp == true) {
      SourceIP.remanent_mask(subnetMask);
      if(find(MonitoredIpAddresses.begin(),MonitoredIpAddresses.end(),SourceIP)
	 !=
	 MonitoredIpAddresses.end()) // i.e. we already saw this IP
	gotSourceIP = true;
      else if (MonitoredIpAddresses.size() < IpListMaxSize) {
	// i.e. we never saw this IP,
	// that's a new one, so we add it to our IP-list,
	// provided there is still place
	MonitoredIpAddresses.push_back(SourceIP);
	gotSourceIP = true;
      }
      else
	IpListMaxSizeReachedAndNewIpWantedToEnterIt = 1;
        // there isn't still place,
        // so we just set the "max-size" flag to 1
        // (ToDo: replace this flag with an exception)
    }

    else {
      SourceIP.remanent_mask(subnetMask);
      if(find(MonitoredIpAddresses.begin(),MonitoredIpAddresses.end(),SourceIP)
	 !=
	 MonitoredIpAddresses.end())
	// i.e. (masked) SourceIP is one of the IPs in the given IpList
	gotSourceIP = true;
    }

    // In the remaining cases (max-size reached, (masked) SourceIP not among
    // the IPs to monitor...), we just let gotSourceIP to its previous value:
    // false.
    // No record will be produced.

    break;


  case IPFIX_TYPEID_destinationIPv4Address:

    if (fieldDataLength != IPFIX_LENGTH_destinationIPv4Address) {
      std::cerr << "Error! Got invalid IPFIX field data (destination IP)! "
		<< "Skipping record.\n";
      return;
    }

    DestIP = IpAddress(fieldData[0], fieldData[1], fieldData[2], fieldData[3]);

    if (MonitorEveryIp == true) {
      DestIP.remanent_mask(subnetMask);
      if ( find(MonitoredIpAddresses.begin(),MonitoredIpAddresses.end(),DestIP)
	   !=
	   MonitoredIpAddresses.end() ) // i.e. we already saw this IP
	gotDestIP = true;
      else if ( MonitoredIpAddresses.size() < IpListMaxSize ) {
	// i.e. we never saw this IP,
	// that's a new one, so we add it to our IpList,
	// provided there is still place
	MonitoredIpAddresses.push_back(DestIP);
	gotDestIP = true;
      }
      else
	IpListMaxSizeReachedAndNewIpWantedToEnterIt = 1;
        // there isn't still place,
        // so we just set the "max-size" flag to 1
        // (ToDo: replace this flag with an exception)
    }

    else {
      DestIP.remanent_mask(subnetMask);
      if ( find(MonitoredIpAddresses.begin(),MonitoredIpAddresses.end(),DestIP)
	   !=
	   MonitoredIpAddresses.end() )
	// i.e. (masked) DestIP is one of the IPs in the given IpList
	gotDestIP = true;
    }

    // In the remaining cases (max-size reached, (masked) DestIP not among
    // the IPs to monitor...), we just let gotDestIP to its previous value:
    // false.
    // No record will be produced.

    break;


  case IPFIX_TYPEID_sourceTransportPort:

    // This case may happen ONLY if:
    // - we subscribed to IPFIX_TYPEID_sourceTransportPort,
    //   i.e. TCP and/or UDP protocols are monitored
    // - AND a TCP or UDP packet was received from the collector
    //
    // This case CANNOT happen when:
    // - only ICMP and/or RAW are monitored (as, in this case,
    //   we do not subscribe to IPFIX_TYPEID_sourceTransportPort);
    //   EVEN IF a TCP or UDP packet was received from the collector

    if (fieldDataLength != IPFIX_LENGTH_sourceTransportPort
	&& fieldDataLength != IPFIX_LENGTH_sourceTransportPort-1) {
      std::cerr << "Error! Got invalid IPFIX field data (source port)! "
		<< "Skipping record.\n";
      return;
    }

    if (fieldDataLength == IPFIX_LENGTH_sourceTransportPort) {
      if ( MonitorAllPorts = true
	   ||
	   MonitoredPorts.end() !=
	   find (MonitoredPorts.begin(), MonitoredPorts.end(),
		 ntohs(*(uint16_t*)fieldData)) )
	gotSourcePort = true;
      // fieldData must be casted into an uint16_t (= unsigned int)
      // as it is a port number
      // (and, also, converted from network order to host order)
    }

    if (fieldDataLength == IPFIX_LENGTH_sourceTransportPort-1) {
      if ( MonitorAllPorts = true
	   || MonitoredPorts.end() !=
	   find (MonitoredPorts.begin(), MonitoredPorts.end(),
		 (uint16_t)*fieldData) )
	gotSourcePort = true;
      // fieldData must be casted into an uint16_t (= unsigned int)
      // as it is a port number
    }

    break;


  case IPFIX_TYPEID_destinationTransportPort:

    // This case may happen ONLY if:
    // - we subscribed to IPFIX_TYPEID_destinationTransportPort,
    //   i.e. TCP and/or UDP protocols are monitored
    // - AND a TCP or UDP packet was received from the collector
    //
    // This case CANNOT happen when:
    // - only ICMP and/or RAW are monitored (as, in this case,
    //   we do not subscribe to IPFIX_TYPEID_destinationTransportPort);
    //   EVEN IF a TCP or UDP packet was received from the collector

    if (fieldDataLength != IPFIX_LENGTH_destinationTransportPort
	&& fieldDataLength != IPFIX_LENGTH_destinationTransportPort-1) {
      std::cerr << "Error! Got invalid IPFIX field data (destination port)! "
		<< "Skipping record.\n";
      return;
    }

    if (fieldDataLength == IPFIX_LENGTH_destinationTransportPort) {
      if ( MonitorAllPorts = true
	   ||
	   MonitoredPorts.end() !=
	   find (MonitoredPorts.begin(), MonitoredPorts.end(),
		 ntohs(*(uint16_t*)fieldData)) )
	gotDestPort = true;
      // fieldData must be casted into an uint16_t (= unsigned int)
      // as it is a port number
      // (and, also, converted from network order to host order)
    }

    if (fieldDataLength == IPFIX_LENGTH_destinationTransportPort-1) {
      if ( MonitorAllPorts = true
	   ||
	   MonitoredPorts.end() !=
	   find (MonitoredPorts.begin(), MonitoredPorts.end(),
		 (uint16_t)*fieldData) )
	gotDestPort = true;
      // fieldData must be casted into an uint16_t (= unsigned int)
      // as it is a port number
    }

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


void StatStore::recordEnd() {

  // Let it be clear that "gotThing" should in fact be read
  // "gotThingThatWeWishedToMonitor"...
  //
  // As explained in StatStore::addFieldData, at the end of an IPFIX record,
  // we are sure to have got every field we subscribed to; however, we are
  // not sure to have got field values we wanted to monitor. For instance,
  // let say we subscribed to the whole IP 5-tuple, namely SourceIP-DestIP
  // Protocol-SourcePort-DestPort, but we want to monitor only TCP protocol
  // on port 80. Suppose a UDP paquet is received; as we subscribed
  // to IPFIX_TYPEID_protocolIdentifier, the corresponding IPFIX record is
  // received from the collector. But as we don't want to monitor UDP,
  // StatStore::addFieldData sets gotProtocol to "false": that's why we
  // should in fact read gotProtocolThatWeWishedToMonitor...
  // And no information is added to our Data map by the two "if" tests.
  //
  // On the other hand, if we receive a TCP paquet on port 80,
  // then StatStore::addFieldData sets gotProtocol (and the others) to "true",
  // and we are sure BOTH "if" tests are true and add information to our
  // Data map.

  if ( gotProtocol == true
       &&
       gotSourceIP == true
       &&
       ( protocol == IPFIX_protocolIdentifier_ICMP
	 || protocol == IPFIX_protocolIdentifier_RAW
	 || protocol == IPFIX_protocolIdentifier_TCP && gotSourcePort == true
	 || protocol == IPFIX_protocolIdentifier_UDP && gotSourcePort == true )
       // port monitoring only for TCP and UDP
       ) {

    Data[SourceIP].packets_out += packet_nb;
    Data[SourceIP].bytes_out   += byte_nb;
    // the mere writting of "Data[SourceIP]" creates the entry <SourceIP,Info>
    // in the map Data, with the Info structure filled with default values
    // (0 for integers); hence we are sure that all 4 fields of the Info
    // structure Data[SourceIP] are initialized with suitable values:
    // at least 0, and more if traffic is noticed

  }

  if ( gotProtocol == true
       &&
       gotDestIP == true
       &&
       ( protocol == IPFIX_protocolIdentifier_ICMP
	 || protocol == IPFIX_protocolIdentifier_RAW
	 || protocol == IPFIX_protocolIdentifier_TCP && gotDestPort == true
	 || protocol == IPFIX_protocolIdentifier_UDP && gotDestPort == true )
       // port monitoring only for TCP and UDP
       ) {

    Data[DestIP].packets_in += packet_nb;
    Data[DestIP].bytes_in   += byte_nb;
    // the mere writting of "Data[DestIP]" creates the entry <DestIP,Info>
    // in the map Data, with the Info structure filled with default values
    // (0 for integers); hence we are sure that all 4 fields of the Info
    // structure Data[DestIP] are initialized with suitable values:
    // at least 0, and more if traffic is noticed

  }

  return;

}


// ========== INITIALISATIONS OF STATIC MEMBERS OF CLASS StatStore ===========

std::map<IpAddress,Info> StatStore::PreviousData;

// even if the following members will be given their actual values
// by the Stat::init() function, we have to provide some initial values
// in the implementation file of the related class;

std::vector<IpAddress> StatStore::MonitoredIpAddresses;

byte StatStore::subnetMask[4] = {0xFF, 0xFF, 0xFF, 0xFF};

bool StatStore::MonitorEveryIp = false;
int StatStore::IpListMaxSize = 0;

std::vector<byte> StatStore::MonitoredProtocols;

std::vector<uint16_t> StatStore::MonitoredPorts;
bool StatStore::MonitorAllPorts = false;

bool StatStore::BeginMonitoring = false;
