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

#include "print-store.h"

PrintStore::PrintStore()
  : sourceAddress(0,0,0,0), destinationAddress(0,0,0,0) {

  flowStart = flowEnd = 0;
  protocol = 0;
  sourcePort = destinationPort = 0;
  nb_packets = nb_bytes = 0;

  recordNumber = 0;
  recordStarted = false;

}

bool PrintStore::recordStart(SourceID) {

  if (recordStarted)
    std::cerr << "PrintStore::recordStart() was called while having a started record!\n";
  recordNumber++;
  return recordStarted = true;

}

void PrintStore::addFieldData(int id, byte * fieldData, int fieldDataLength,
			      EnterpriseNo eid) {

  // we subscribed to (see Print::init()):
  // - IPFIX_TYPEID_flowStartSeconds and IPFIX_TYPEID_flowEndSeconds
  // - IPFIX_TYPEID_sourceIPv4Address and IPFIX_TYPEID_destinationIPv4Address
  // - IPFIX_TYPEID_protocolIdentifier
  // - IPFIX_TYPEID_sourceTransportPort and
  //   IPFIX_TYPEID_destinationTransportPort
  // - IPFIX_TYPEID_packetDeltaCount and IPFIX_TYPEID_octetDeltaCount

  // addFieldData will be executed until there are no more fieldData
  // in the current IPFIX record; so we are sure to get everything
  // we subscribed to (so don't get worried because of
  // the "breaks" in the "switch" loop hereafter)

  switch (id) {

  case IPFIX_TYPEID_flowStartSeconds:

    if (fieldDataLength != IPFIX_LENGTH_flowCreationTime) {
      std::cerr << "Error! Got invalid IPFIX field data (flow start)! "
		<< "Skipping record.\n";
      return;
    }
    flowStart = ntohl(uint32_t(*fieldData));
    break;

  case IPFIX_TYPEID_flowEndSeconds:

    if (fieldDataLength != IPFIX_LENGTH_flowEndTime) {
      std::cerr << "Error! Got invalid IPFIX field data (flow end)! "
		<< "Skipping record.\n";
      return;
    }
    flowEnd = ntohl(uint32_t(*fieldData));
    break;

  case IPFIX_TYPEID_protocolIdentifier:

    if (fieldDataLength != IPFIX_LENGTH_protocolIdentifier) {
      std::cerr << "Error! Got invalid IPFIX field data (protocol)! "
		<< "Skipping record.\n";
      return;
    }
    protocol = uint16_t(*fieldData);
    // *fielData is a protocol number, so is 1 byte (= uint8_t) long
    // hence the cast into an uint16_t (= unsigned int) as we want to print it
    break;

  case IPFIX_TYPEID_sourceIPv4Address:

    if (fieldDataLength != IPFIX_LENGTH_sourceIPv4Address) {
      std::cerr << "Error! Got invalid IPFIX field data (source IP)! "
		<< "Skipping record.\n";
      return;
    }
    sourceAddress.setAddress(fieldData[0], fieldData[1],
			     fieldData[2], fieldData[3]);
    break;

  case IPFIX_TYPEID_destinationIPv4Address:

    if (fieldDataLength != IPFIX_LENGTH_destinationIPv4Address) {
      std::cerr << "Error! Got invalid IPFIX field data (destination IP)! "
		<< "Skipping record.\n";
      return;
    }
    destinationAddress.setAddress(fieldData[0], fieldData[1],
				  fieldData[2], fieldData[3]);
    break;

  case IPFIX_TYPEID_sourceTransportPort:

    if (fieldDataLength != IPFIX_LENGTH_sourceTransportPort
	&& fieldDataLength != IPFIX_LENGTH_sourceTransportPort-1) {
      std::cerr << "Error! Got invalid IPFIX field data (source port)! "
		<< "Skipping record.\n";
      return;
    }
    if (fieldDataLength == IPFIX_LENGTH_sourceTransportPort)
      sourcePort = ntohs(uint16_t(*fieldData));
    // fieldData must be casted into an uint16_t (= unsigned int)
    // as it is a port number
    // (and, also, converted from network order to host order)
    if (fieldDataLength == IPFIX_LENGTH_sourceTransportPort-1)
      sourcePort = uint16_t(*fieldData);
    // fieldData must be casted into an uint16_t (= unsigned int
    // as it is a port number
    break;


  case IPFIX_TYPEID_destinationTransportPort:

    if (fieldDataLength != IPFIX_LENGTH_destinationTransportPort
	&& fieldDataLength != IPFIX_LENGTH_destinationTransportPort-1) {
      std::cerr << "Error! Got invalid IPFIX field data (destination port)! "
		<< "Skipping record.\n";
      return;
    }
    if (fieldDataLength == IPFIX_LENGTH_destinationTransportPort)
      destinationPort = ntohs(uint16_t(*fieldData));
    // fieldData must be casted into an uint16_t (= unsigned int)
    // as it is a port number
    // (and, also, converted from network order to host order)
    if (fieldDataLength == IPFIX_LENGTH_destinationTransportPort-1)
      destinationPort = uint16_t(*fieldData);
    // fieldData must be casted into an uint16_t (= unsigned int)
    // as it is a port number
    break;

  case IPFIX_TYPEID_packetDeltaCount:

    if (fieldDataLength != IPFIX_LENGTH_packetDeltaCount) {
      std::cerr << "Error! Got invalid IPFIX field data (#packets)! "
		<< "Skipping record.\n";
      return;
    }
    nb_packets = ntohll(uint64_t(*fieldData));
    break;

  case IPFIX_TYPEID_octetDeltaCount:

    if (fieldDataLength != IPFIX_LENGTH_octetDeltaCount) {
      std::cerr << "Error! Got invalid IPFIX field data (#bytes)! "
		<< "Skipping record.\n";
      return;
    }
    nb_bytes = ntohll(uint64_t(*fieldData));
    break;

  default:

    std::cerr
      << "Warning! Got unknown record in StatStore::addFieldData(...)!\n";

  }

  return;

}

void PrintStore::recordEnd() {

  if (!recordStarted)
    std::cerr
      << "PrintStore::recordEnd() was called before starting a record!\n";
  if (recordNumber > 1)
    std::cerr << "PrintStore object has more than one record!\n";

  return;

}
