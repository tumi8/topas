/**************************************************************************/
/*                                                                        */
/*   Copyright (C) 2006-2007  Nico Weber                                  */
/*                                                                        */
/*        structs for the ethernet/ip/tcp/udp packet information          */
/*        as well as some checksum calculation algorithms                 */
/*        were taken from text2pcap.c out of the wireshark-project        */
/*        (former ethereal) and "c++ified"                                */
/*                                                                        */
/*   Credits for text2pcap.c:                                             */
/*   (c) Copyright 2001 Ashok Narayanan <ashokn@cisco.com>                */
/*                                                                        */
/*   $Id: pcappacket.h,v 1.3 2007/01/19 12:11:18 braunl Exp $                 */
/*                                                                        */
/*   Wireshark - Network traffic analyzer                                 */
/*   By Gerald Combs <gerald@wireshark.org>                               */
/*   Copyright 1998 Gerald Combs                                          */
/*                                                                        */
/*   This program is free software; you can redistribute it and/or        */
/*   modify it under the terms of the GNU General Public License          */
/*   as published by the Free Software Foundation; either version 2       */
/*   of the License, or (at your option) any later version.               */
/*                                                                        */
/*   This program is distributed in the hope that it will be useful,      */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*   GNU General Public License for more details.                         */
/*                                                                        */
/*   You should have received a copy of the GNU General Public License    */
/*   along with this program; if not, write to the Free Software          */
/*   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA            */
/*   111-1307, USA.                                                       */
/*                                                                        */
/**************************************************************************/

#ifndef _PCAPPACKET_H_
#define _PCAPPACKET_H_

#include <glib.h>

/**\brief Class for the main layout of a network packet
 *
 * Contains the main different structs of a network packet. It is used by the storage class to store one packet.
 */

class PcapPacket {
	
public:

	PcapPacket();	///< default constructor
	~PcapPacket();	///< default destructor
	
	bool hdr_udp; ///< used to determine if UDP data has been stored
	bool hdr_tcp; ///< used to determine if TCP data has been stored	
	
	
	guint16 hdr_dest_port; ///< Transport protocol destination port is stored in here
	guint16 hdr_src_port; ///< Transport protocol source port is stored in here

	guint32 ts_sec; 	///< unix time when the packet was recieved
	static guint32 ts_usec;	///< part of the unix time
	static char *ts_fmt;

	/**\brief Struct of the typical ethernet header
	 */

	struct hdr_ethernet_t {
	    guint8  dest_addr[6];
	    guint8  src_addr[6];
	    guint16 l3pid;
	};

	static hdr_ethernet_t HDR_ETHERNET; ///< the dummy  ethernet header
	
	/**\brief Struct of the typical IPv4 header
	 */
	
	struct hdr_ip_t{
	    guint8  ver_hdrlen;
	    guint8  dscp;
	    guint16 packet_length;
	    guint16 identification;
	    guint8  flags;
	    guint8  fragment;
	    guint8  ttl;
	    guint8  protocol;
	    guint16 hdr_checksum;
	    guint32 src_addr;
	    guint32 dest_addr;
	
	    hdr_ip_t() : ver_hdrlen(0x45),dscp(0),packet_length(0),identification(0),flags(0),fragment(0),ttl(0),protocol(0),hdr_checksum(0),src_addr(0),dest_addr(0){}; ///< default constructor to set default values
	};
	

	hdr_ip_t HDR_IP ; ///< IPv4 header of the packet
	
        /**\brief Struct of the typical UDP header
         */
	
	struct hdr_udp_t {
	    guint16 source_port;
	    guint16 dest_port;
	    guint16 length;
	    guint16 checksum;
	
	    hdr_udp_t() : source_port(0),dest_port(0),length(12),checksum(0) {}; ///< default constructor to set default values
	};
	
	hdr_udp_t HDR_UDP; ///< UDP Header of the packet
	
	/**\brief Struct of the typical TCP header
	 */
	
	struct hdr_tcp_t {
	    guint16 source_port;
	    guint16 dest_port;
	    guint32 seq_num;
	    guint32 ack_num;
	    guint8  hdr_length;
	    guint8  flags;
	    guint16 window;
	    guint16 checksum;
	    guint16 urg;
	
	
	    hdr_tcp_t(): seq_num(0),ack_num(0),hdr_length(0x50),flags(0), window(0), checksum(0), urg(0) {}; ///< default constructor to set default values
	};
	
	hdr_tcp_t HDR_TCP; ///< TCP header of the packet
	
	//Pointer for Data 
	
	char *iphps_p; ///< Pointer to IPHeader payload
	int   iphps_size; ///< Size of stored IPHeader payload
	char *ippps_p;	///< Pointer to IPPacket payload
	int ippps_size;	///< Size of stored IPPacket payload

private:

};
#endif
