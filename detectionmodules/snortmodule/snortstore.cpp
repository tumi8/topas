/**************************************************************************/
/*   Copyright (C) 2006-2007 Nico Weber                                   */
/*                                                                        */
/*   This library is free software; you can redistribute it and/or        */
/*   modify it under the terms of the GNU Lesser General Public           */
/*   License as published by the Free Software Foundation; either         */
/*   version 2.1 of the License, or (at your option) any later version.   */
/*                                                                        */
/*   This library is distributed in the hope that it will be useful,      */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*   Lesser General Public License for more details.                      */
/*                                                                        */
/*   You should have received a copy of the GNU Lesser General Public     */
/*   License along with this library; if not, write to the Free Software  */
/*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,           */
/*   MA  02110-1301, USA                                                  */
/**************************************************************************/

/**\file snortstore.cpp
 *
 * Functions for storage of the packet
 */

#include "snortstore.h"


#include <concentrator/ipfix.h>


#include <cstring>
#include <iostream>
#include <cassert>
#include <algorithm>

SnortStore::SnortStore():is_valid(false) {
}

SnortStore::~SnortStore() {
delete packet;
}

void SnortStore::addFieldData(int id, byte* fieldData, int fieldDataLength, EnterpriseNo eid) {
	switch (id) {
		// IP-Header
		case IPFIX_TYPEID_sourceIPv4Address:
			bcopy(fieldData, &packet->HDR_IP.src_addr, fieldDataLength);
			break;
                case IPFIX_TYPEID_destinationIPv4Address:
			bcopy(fieldData, &packet->HDR_IP.dest_addr, fieldDataLength);
                        break;
		case IPFIX_TYPEID_classOfServiceIPv4:
			bcopy(fieldData, &packet->HDR_IP.dscp, fieldDataLength);
			break;
		case IPFIX_TYPEID_identificationV4: 
			bcopy(fieldData, &packet->HDR_IP.identification, fieldDataLength);
			break;  
		case IPFIX_TYPEID_ipTimeToLive:
			bcopy(fieldData, &packet->HDR_IP.ttl, fieldDataLength);
			break;
		case IPFIX_TYPEID_totalLengthIPv4:
			bcopy(fieldData, &packet->HDR_IP.packet_length, fieldDataLength);				
			break;
		case IPFIX_TYPEID_protocolIdentifier:
			bcopy(fieldData, &packet->HDR_IP.protocol, fieldDataLength);
			break;
			
		//Ports
		case IPFIX_TYPEID_destinationTransportPort:
			bcopy(fieldData, &packet->hdr_dest_port, fieldDataLength);
                        break;
		case IPFIX_TYPEID_sourceTransportPort: 
			bcopy(fieldData, &packet->hdr_src_port, fieldDataLength);
			break;
								
		//TCP
	        case IPFIX_TYPEID_tcpControlBits:
		        packet->hdr_tcp=true;
			bcopy(fieldData, &packet->HDR_TCP.flags, fieldDataLength);
		        break;
		case IPFIX_TYPEID_tcpSequenceNumber:
			packet->hdr_tcp=true;
			bcopy(fieldData, &packet->HDR_TCP.seq_num, fieldDataLength);
			break;
		case IPFIX_TYPEID_tcpAcknowledgementNumber:
			packet->hdr_tcp=true;
			bcopy(fieldData, &packet->HDR_TCP.ack_num, fieldDataLength);
			break;
		case IPFIX_TYPEID_tcpWindowSize:
			packet->hdr_tcp=true;
			bcopy(fieldData, &packet->HDR_TCP.window, fieldDataLength);
			break;
		case 187: // UrgentPointer
			packet->hdr_tcp=true;
			bcopy(fieldData, &packet->HDR_TCP.urg, fieldDataLength);
			break;
		case IPFIX_TYPEID_tcpSourcePort:
			packet->hdr_tcp=true;
			bcopy(fieldData, &packet->hdr_src_port, fieldDataLength);
			break;
		case IPFIX_TYPEID_tcpDestinationPort:
			packet->hdr_tcp=true;
			bcopy(fieldData, &packet->hdr_src_port, fieldDataLength);
			break;
									
		//UDP
		case 	IPFIX_TYPEID_udpSourcePort:
			packet->hdr_udp=true;
			bcopy(fieldData, &packet->hdr_src_port, fieldDataLength);
			break;
		case 	IPFIX_TYPEID_udpDestinationPort:
			packet->hdr_udp=true;
			bcopy(fieldData, &packet->hdr_src_port, fieldDataLength);
			break;
		
		//DATA
		case 313: //PSAMP_TYPEID_ipHeaderPacketSection 
			packet->iphps_p = new char[fieldDataLength];
			packet->iphps_size=fieldDataLength;
			bcopy(fieldData, packet->iphps_p, fieldDataLength);
			break;
		case 314: //PSAMP_TYPEID_ipPayloadPacketSection  
			packet->ippps_p = new char[fieldDataLength];
			packet->ippps_size=fieldDataLength;
			bcopy(fieldData, packet->ippps_p, fieldDataLength);
			break;			

		//Timestamps
		case	IPFIX_TYPEID_flowStartSeconds:
			packet->ts_sec = (uint32_t)fieldToInt(fieldData, fieldDataLength);
			break;
		case	IPFIX_TYPEID_flowStartMicroSeconds:
			packet->ts_usec = (uint32_t)fieldToInt(fieldData, fieldDataLength);
			break;

		//Rest ;)
		default:
			break;
	}
}

bool SnortStore::recordStart(SourceID sourceid) {
	is_valid=true;
	packet = new PcapPacket;
	packet->ts_sec =time(NULL);
	return true;

}

void SnortStore::recordEnd() {
}

PcapPacket* SnortStore::get_record(){
	return packet;
}
