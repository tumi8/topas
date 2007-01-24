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

#ifndef _PCAPWRITER_H_
#define _PCAPWRITER_H_

#include <glib.h>
#include "pcappacket.h"
#include <stdio.h>

#define PCAP_MAGIC 0xa1b2c3d4 ///< Special PCAP_MAGIC to show what file format we are using
#define PADDING 2048

/**\brief Does all the logic, calculate checksums and write the packet into the fifo
 *
 */

class pcapwriter {
	
public:
	pcapwriter(); ///< default constructor
	~pcapwriter();	///< default destructor
	
	void writefileheader(); ///< writes inital fileheader to the outputfile or fifo
	void writepacket(PcapPacket* record);	///< process the record and write it 
	void writedummypacket();	///< writes a dummy packet to the fifo
	
	/**\brief Inits the writer
	 *
	 * \param output_file is the output file/fifo the writer should hook on
	 * \param b1 calculate IPHeader checksum
	 * \param b2 calculate Transportheader checksum
	 */
	
	void init(FILE* output_file, bool b1, bool b2);
	
	unsigned long get_packets_written(); ///< returns number of already written packets
	unsigned long get_packets_read(); ///< returns number of read packets

private:
	FILE *output_file;	
	
	unsigned long packets_read;
	unsigned long packets_written;
	unsigned long pcap_link_type;
	
	bool calc_iphcs;
	bool calc_thcs;
	
	
	char tempbuf[64];
	unsigned char padding[PADDING];

	guint16 in_checksum (void *buf, unsigned long count);

	/*
	 * The CRC32C code is taken from draft-ietf-tsvwg-sctpcsum-01.txt.
	 *  That code is copyrighted by D. Otis and has been modified.
	*/
	
	static guint32 crc_c[256];
	guint32 crc32c(const guint8* buf, unsigned int len, guint32 crc32_init);
	guint32 finalize_crc32c(guint32 crc32);
					
	/**\brief "libpcap" file header 
	 */
	struct pcap_hdr {
    		guint32     magic;          ///< magic 
    		guint16     version_major;  ///< major version number
    		guint16     version_minor;  ///< minor version number
    		guint32     thiszone;       ///< GMT to local correction
    		guint32     sigfigs;        ///< accuracy of timestamps
    		guint32     snaplen;        ///< max length of captured packets, in octets
    		guint32     network;        ///< data link type
	};
	        
	/**\brief "libpcap" record header. 
	 */
	struct pcaprec_hdr {
		gint32      ts_sec;         ///< timestamp seconds
		guint32     ts_usec;        ///< timestamp microseconds
		guint32     incl_len;       ///< number of octets of packet saved in file
		guint32     orig_len;       ///< actual length of packet
	};
        
	/**\brief pseudo header for checksum calculation 
	 */
	
	struct pseudoh_t { 
		guint32 src_addr;
		guint32 dest_addr;
		guint8  zero;
		guint8  protocol;
		guint16 length;
	};
        
	pseudoh_t pseudoh;

};
#endif
