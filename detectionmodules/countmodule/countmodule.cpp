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

#include "countmodule.h"
#include <concentrator/ipfix_names.h>

#include <iostream>


bool CountModule::verbose = false;

/* Constructor and destructor */
CountModule::CountModule(const std::string& configfile)
: DetectionBase<CountStore>(configfile), octetThreshold(0), packetThreshold(0), flowThreshold(0)
{
    /* signal handlers */
    if (signal(SIGTERM, sigTerm) == SIG_ERR) {
	msgStr.print(MsgStream::ERROR, "Couldn't install signal handler for SIGTERM.");
    } 
    if (signal(SIGINT, sigInt) == SIG_ERR) {
	msgStr.print(MsgStream::ERROR, "Couldn't install signal handler for SIGINT.");
    } 	

    init(configfile);
}

CountModule::~CountModule() {

}

/* Initialization and update*/
void CountModule::init(const std::string& configfile)
{
    /* Default values */
    alarm = 10;
    unsigned bfSize = 1024;
    unsigned bfHF = 3;
    char filename[] = "countmodule.txt";

    XMLConfObj config = XMLConfObj(configfile, XMLConfObj::XML_FILE);
    
    if(config.nodeExists("preferences"))
    {
	config.enterNode("preferences");

	if(config.nodeExists("verbose"))
	    if(config.getValue("verbose") != "false")
	    {
		CountModule::verbose = true;
		msgStr.setLevel(MsgStream::INFO);
	    }

	if(config.nodeExists("debug"))
	    if(config.getValue("debug") != "false")
	    {
		CountModule::verbose = true;
		msgStr.setLevel(MsgStream::DEBUG);
	    }

	msgStr.print(MsgStream::INFO, "Starting with parameters:");

	if(config.nodeExists("output_file"))
	    outfile.open(config.getValue("output_file").c_str());
	else
	    outfile.open(filename);
	if(!outfile) 
	{
	    msgStr << MsgStream::ERROR << "Could not open output file " << filename << "." << MsgStream::endl;
	    stop();
	}
	msgStr << MsgStream::INFO << "Output file: " << filename << MsgStream::endl;

	if(config.nodeExists("alarm_time"))
	    alarm = atoi(config.getValue("alarm_time").c_str());
	setAlarmTime(alarm);
	msgStr << MsgStream::INFO << "Test interval: " << alarm << " seconds" << MsgStream::endl;

	if(config.nodeExists("accept_source_ids"))
	{
	    std::string str = config.getValue("accept_source_ids");
	    if(str.size()>0)
	    {
		msgStr << MsgStream::INFO << "Accepted source ids: " << str << MsgStream::endl;
		unsigned startpos = 0, endpos = 0;
		do {
		    endpos = str.find(',', endpos);
		    if (endpos == std::string::npos) {
			subscribeSourceId(atoi((str.substr(startpos)).c_str()));
			break;
		    }
		    subscribeSourceId(atoi((str.substr(startpos, endpos-startpos)).c_str()));
		    endpos++;
		}
		while(true);
	    }
	}

	if(config.nodeExists("octet_threshold"))
	    octetThreshold = atoi(config.getValue("octet_threshold").c_str());
	if(config.nodeExists("packet_threshold"))
	    packetThreshold = atoi(config.getValue("packet_threshold").c_str());
	if(config.nodeExists("flow_threshold"))
	    flowThreshold = atoi(config.getValue("flow_threshold").c_str());
	msgStr << MsgStream::INFO << "Thresholds: " << octetThreshold << " octets, " << packetThreshold << " packets, "
	    << flowThreshold << " flows" << MsgStream::endl;

	config.leaveNode();
    }

    if(config.nodeExists("counting"))
    {
	config.enterNode("counting");

	if(config.nodeExists("bf_size"))
	    bfSize = atoi(config.getValue("bf_size").c_str());
	if(config.nodeExists("bf_hashfunctions"))
	    bfHF = atoi(config.getValue("bf_hashfunctions").c_str());
	CountStore::init(bfSize, bfHF);
	msgStr << MsgStream::INFO << "Bloomfilter: " << bfSize << " bits, " << bfHF << " hash functions" << MsgStream::endl;

	msgStr << MsgStream::INFO << "Counting per ";
	if(config.nodeExists("count_per_src_ip"))
	    if(config.getValue("count_per_src_ip") != "false") 
	    {
		CountStore::countPerSrcIp = true;
		msgStr << "src addr, ";
	    }
	if(config.nodeExists("count_per_dst_ip"))
	    if(config.getValue("count_per_dst_ip") != "false")
	    {
		CountStore::countPerDstIp = true;
		msgStr << "dst addr, ";
	    }
	if(config.nodeExists("count_per_src_port"))
	    if(config.getValue("count_per_src_port") != "false")
	    {
		CountStore::countPerSrcPort = true;
		msgStr << "src port, ";
	    }
	if(config.nodeExists("count_per_dst_port"))
	    if(config.getValue("count_per_dst_port") != "false")
	    {
		CountStore::countPerDstPort = true;
		msgStr << "dst port";
	    }
	msgStr << MsgStream::endl;
    }

#ifdef IDMEF_SUPPORT_ENABLED
    /* register module */
    registerModule("countmodule");
#endif

    subscribeTypeId(IPFIX_TYPEID_sourceIPv4Address);
    subscribeTypeId(IPFIX_TYPEID_sourceTransportPort);
    subscribeTypeId(IPFIX_TYPEID_destinationIPv4Address);
    subscribeTypeId(IPFIX_TYPEID_destinationTransportPort);
    subscribeTypeId(IPFIX_TYPEID_protocolIdentifier);
    subscribeTypeId(IPFIX_TYPEID_octetDeltaCount);
    subscribeTypeId(IPFIX_TYPEID_packetDeltaCount);
}

#ifdef IDMEF_SUPPORT_ENABLED
void CountModule::update(XMLConfObj* xmlObj)
{
    if (xmlObj->nodeExists("stop")) {
	msgStr.print(MsgStream::INFO, "Update: Stopping module.");
	stop();
    } else if (xmlObj->nodeExists("restart")) {
	msgStr.print(MsgStream::INFO, "Update: Restarting module.");
	restart();
    } else if (xmlObj->nodeExists("config")) {
	msgStr.print(MsgStream::INFO, "Update: Changing module configuration.");
	xmlObj->enterNode("config");
	if(xmlObj->nodeExists("count_per_src_ip")) {
	    if(xmlObj->getValue("count_per_src_ip") != "false")
		CountStore::countPerSrcIp = true;
	    else
		CountStore::countPerSrcIp = false; }
	if(xmlObj->nodeExists("count_per_dst_ip")) {
	    if(xmlObj->getValue("count_per_dst_ip") != "false")
		CountStore::countPerDstIp = true;
	    else
		CountStore::countPerDstIp = false; }
	if(xmlObj->nodeExists("count_per_src_port")) {
	    if(xmlObj->getValue("count_per_src_port") != "false")
		CountStore::countPerSrcPort = true;
	    else
		CountStore::countPerSrcPort = false; }
	if(xmlObj->nodeExists("count_per_dst_port")) {
	    if(xmlObj->getValue("count_per_dst_port") != "false")
		CountStore::countPerDstPort = true;
	    else
		CountStore::countPerDstPort = false; }
	if(xmlObj->nodeExists("octet_threshold"))
	    octetThreshold = atoi(xmlObj->getValue("octet_threshold").c_str());
	if(xmlObj->nodeExists("packet_threshold"))
	    packetThreshold = atoi(xmlObj->getValue("packet_threshold").c_str());
	if(xmlObj->nodeExists("flow_threshold"))
	    flowThreshold = atoi(xmlObj->getValue("flow_threshold").c_str());
    } else { // add your commands here
	msgStr.print(MsgStream::INFO, "Update: Unsupported operation.");
    }
}
#endif

/* Test */
void CountModule::test(CountStore* store) 
{
#ifdef IDMEF_SUPPORT_ENABLED
    std::stringstream portStr, protoStr, ocStr, pcStr, fcStr, orStr, prStr;
    IdmefMessage& idmefMessage = getNewIdmefMessage("Countmodule", "threshold detection");
#endif

    msgStr.print(MsgStream::INFO, "Generating report...");
    outfile << "******************** Report *********************" << std::endl;
    outfile << "thresholds: octets>=" << octetThreshold << " packets>=" << packetThreshold << " flows>=" << flowThreshold << std::endl;

    if(CountStore::countPerSrcIp)
    {
	outfile << "per source IP address:" << std::endl;
	for (CountStore::IpCountMap::const_iterator i = store->srcIpCounts.begin(); i != store->srcIpCounts.end(); ++i) 
	{
	    if(checkThresholds(i->second))
	    {
		outfile << i->first << " \to:" << i->second.octetCount << " \tp:" << i->second.packetCount << " \tf:" << i->second.flowCount << std::endl;

#ifdef IDMEF_SUPPORT_ENABLED
		idmefMessage = getNewIdmefMessage();
		idmefMessage.createSourceNode("no", "ipv4-addr", i->first.toString(), "255.255.255.255");
		ocStr.str(""); ocStr << i->second.octetCount;
		pcStr.str(""); pcStr << i->second.packetCount;
		fcStr.str(""); fcStr << i->second.flowCount;
		orStr.str(""); orStr << (unsigned)(i->second.octetCount/alarm);
		prStr.str(""); prStr << (unsigned)(i->second.packetCount/alarm);
		idmefMessage.createExtStatisticsNode(ocStr.str(), pcStr.str(), fcStr.str(), orStr.str(), prStr.str(), "");
		sendIdmefMessage("Dummy", idmefMessage);
#endif       
	    }   
	}
    }

    if(CountStore::countPerDstIp)
    {
	outfile << "per destination IP address:" << std::endl;
	for (CountStore::IpCountMap::const_iterator i = store->dstIpCounts.begin(); i != store->dstIpCounts.end(); ++i) 
	{
	    if(checkThresholds(i->second))
	    {
		outfile << i->first << " \to:" << i->second.octetCount << " \tp:" << i->second.packetCount << " \tf:" << i->second.flowCount << std::endl;

#ifdef IDMEF_SUPPORT_ENABLED
		idmefMessage = getNewIdmefMessage();
		idmefMessage.createTargetNode("no", "ipv4-addr", i->first.toString(), "255.255.255.255");
		ocStr.str(""); ocStr << i->second.octetCount;
		pcStr.str(""); pcStr << i->second.packetCount;
		fcStr.str(""); fcStr << i->second.flowCount;
		orStr.str(""); orStr << (unsigned)(i->second.octetCount/alarm);
		prStr.str(""); prStr << (unsigned)(i->second.packetCount/alarm);
		idmefMessage.createExtStatisticsNode(ocStr.str(), pcStr.str(), fcStr.str(), orStr.str(), prStr.str(), "");
		sendIdmefMessage("Dummy", idmefMessage);
#endif       
	    }
	}
    }

    if(CountStore::countPerSrcPort)
    {
	outfile << "per source protocol.port:" << std::endl;
	for (CountStore::PortCountMap::const_iterator i = store->srcPortCounts.begin(); i != store->srcPortCounts.end(); ++i) 
	{
	    if(checkThresholds(i->second))
	    {
		outfile << (i->first >> 16) << "." << (0x0000FFFF & i->first) << " \to:" << i->second.octetCount << " \tp:" << i->second.packetCount << " \tf:" << i->second.flowCount << std::endl;

#ifdef IDMEF_SUPPORT_ENABLED
		idmefMessage = getNewIdmefMessage();
		//idmefMessage.createSourceNode("unknown", "ipv4-addr", "0.0.0.0", "0.0.0.0");
		portStr.str(""); protoStr << (0x0000FFFF & i->first);
		protoStr.str(""); protoStr << (i->first >> 16);
		idmefMessage.createServiceNode("Source", "", portStr.str(), "", protoStr.str()); 
		ocStr.str(""); ocStr << i->second.octetCount;
		pcStr.str(""); pcStr << i->second.packetCount;
		fcStr.str(""); fcStr << i->second.flowCount;
		orStr.str(""); orStr << (unsigned)(i->second.octetCount/alarm);
		prStr.str(""); prStr << (unsigned)(i->second.packetCount/alarm);
		idmefMessage.createExtStatisticsNode(ocStr.str(), pcStr.str(), fcStr.str(), orStr.str(), prStr.str(), "");
		sendIdmefMessage("Dummy", idmefMessage);
#endif       
	    }
	}
    }

    if(CountStore::countPerDstPort)
    {
	outfile << "per destination protocol.port:" << std::endl;
	for (CountStore::PortCountMap::const_iterator i = store->dstPortCounts.begin(); i != store->dstPortCounts.end(); ++i) 
	{
	    if(checkThresholds(i->second))
	    {
		outfile << (i->first >> 16) << "." << (0x0000FFFF & i->first) << " \to:" << i->second.octetCount << " \tp:" << i->second.packetCount << " \tf:" << i->second.flowCount << std::endl;

#ifdef IDMEF_SUPPORT_ENABLED
		idmefMessage = getNewIdmefMessage();
		//idmefMessage.createTargetNode("unknown", "ipv4-addr", "0.0.0.0", "0.0.0.0");
		portStr.str(""); protoStr << (0x0000FFFF & i->first);
		protoStr.str(""); protoStr << (i->first >> 16);
		idmefMessage.createServiceNode("Target", "", portStr.str(), "", protoStr.str()); 
		ocStr.str(""); ocStr << i->second.octetCount;
		pcStr.str(""); pcStr << i->second.packetCount;
		fcStr.str(""); fcStr << i->second.flowCount;
		orStr.str(""); orStr << (unsigned)(i->second.octetCount/alarm);
		prStr.str(""); prStr << (unsigned)(i->second.packetCount/alarm);
		idmefMessage.createExtStatisticsNode(ocStr.str(), pcStr.str(), fcStr.str(), orStr.str(), prStr.str(), "");
		sendIdmefMessage("Dummy", idmefMessage);
#endif       
	    }
	}
    }

    outfile << "********************* End ***********************" << std::endl;

    delete store;
}

bool CountModule::checkThresholds(const Counters& count)
{
    return ((count.octetCount >= octetThreshold) || (count.packetCount >= packetThreshold) || (count.flowCount >= flowThreshold));
}

void CountModule::sigTerm(int signum)
{
    stop();
}

void CountModule::sigInt(int signum)
{
    stop();
}
