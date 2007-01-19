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

#include "print-main.h"

Print::Print(const std::string & configfile)
  : DetectionBase< PrintStore,
		   UnbufferedFilesInputPolicy<SemShmNotifier, PrintStore> >(configfile) {
  init(configfile);
}

void Print::init(const std::string & configfile) {

  ConfObj * config;
  config = new ConfObj(configfile);

  // extracting output file's name from configfile
  if (NULL != config->getValue("preferences", "output_file")) {
    outfile.open(config->getValue("preferences", "output_file"));
    if (!outfile) {
      std::cerr << "Error: could not open output file "
		<< config->getValue("preferences", "output_file") << ". "
		<< "Check if you have enough rights to create or write to it. "
		<< "Exiting.\n";
      exit(-1);
    }
  }
  else {
    std::cerr <<"Error! No output_file parameter defined in XML config file!\n"
	      <<"Please give one and restart. Exiting.\n";
    exit(-1);
  }

  // set alarm time to 0: the test() method will be called
  // as soon as a record is received ('real-time monitoring')
  // (a PrintStore object will then contain no more than one record)
  setAlarmTime(0);

  // monitored values
  subscribeTypeId(IPFIX_TYPEID_flowStartSeconds);
  subscribeTypeId(IPFIX_TYPEID_flowEndSeconds);
  subscribeTypeId(IPFIX_TYPEID_sourceIPv4Address);
  subscribeTypeId(IPFIX_TYPEID_destinationIPv4Address);
  subscribeTypeId(IPFIX_TYPEID_protocolIdentifier);
  subscribeTypeId(IPFIX_TYPEID_sourceTransportPort);
  subscribeTypeId(IPFIX_TYPEID_destinationTransportPort);
  subscribeTypeId(IPFIX_TYPEID_packetDeltaCount);
  subscribeTypeId(IPFIX_TYPEID_octetDeltaCount);

  // beginning sending output to outfile
  outfile << "|Flow Start|Flow End  |Source IP      |Destination IP |Proto|SrcPort|DestPort|Packets   |Bytes     |\n"
	  << "|----------|----------|---------------|---------------|-----|-------|--------|----------|----------|\n"
	  << std::flush;

}

void Print::test(PrintStore * store) {

  int len;

  // output Flow Start
  std::ostringstream flow_start;
  flow_start << '|' << store->flowStart;
  len = flow_start.str().length();

  outfile << store->flowStart;
  for (int i = 0; i != 10 - len; i++)
    outfile << ' ';

  // output Flow End
  std::ostringstream flow_end;
  flow_end << store->flowEnd;
  len = flow_end.str().length();

  outfile << '|' << store->flowEnd;
  for (int i = 0; i != 10 - len; i++)
    outfile << ' ';

  // output Source IP
  len = store->sourceAddress.toString().length();

  outfile << '|' << store->sourceAddress.toString();
  for (int i = 0; i != 15 - len; i++)
    outfile << ' ';

  // output Destination IP
  len = store->destinationAddress.toString().length();

  outfile << '|' << store->destinationAddress.toString();
  for (int i = 0; i != 15 - len; i++)
    outfile << ' ';

  // output Protocol
  std::ostringstream protocol;
  protocol << store->protocol;
  len = protocol.str().length();

  outfile << '|' << store->protocol;
  for (int i = 0; i != 5 - len; i++)
    outfile << ' ';

  // output Source Port
  std::ostringstream source_port;
  source_port << store->sourcePort;
  len = source_port.str().length();

  outfile << '|' << store->sourcePort;
  for (int i = 0; i != 7 - len; i++)
    outfile << ' ';

  // output Destination Port
  std::ostringstream dest_port;
  dest_port << store->destinationPort;
  len = dest_port.str().length();

  outfile << '|' << store->destinationPort;
  for (int i = 0; i != 8 - len; i++)
    outfile << ' ';

  // output Packet number
  std::ostringstream nb_packets;
  nb_packets << store->nb_packets;
  len = nb_packets.str().length();

  outfile << '|' << store->nb_packets;
  for (int i = 0; i != 10 - len; i++)
    outfile << ' ';

  // output Packet number
  std::ostringstream nb_bytes;
  nb_bytes << store->nb_bytes;
  len = nb_bytes.str().length();

  outfile << '|' << store->nb_bytes;
  for (int i = 0; i != 10 - len; i++)
    outfile << ' ';

  // add \n and flush
  outfile << std::endl;

  /* don't forget to free the store-object */
  delete store;

}
