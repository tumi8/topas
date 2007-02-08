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
/*   $Id: pcappacket.cpp,v 1.3 2007/01/19 12:11:18 braunl Exp $                 */
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

#include "pcappacket.h"


PcapPacket::PcapPacket() : iphps_p(0), iphps_size(0), ippps_p(0), ippps_size(0), hdr_tcp(false), hdr_udp(false), ts_sec(0) {};
PcapPacket::~PcapPacket(){
delete iphps_p;
delete ippps_p;
};

uint32_t PcapPacket::ts_usec = 0;
char *PcapPacket::ts_fmt = NULL;
PcapPacket::hdr_ethernet_t PcapPacket::HDR_ETHERNET = {
    {0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    0}; ///< The ethernet header is initialized with dummy values






