/**************************************************************************/
/*    Copyright (C) 2007 Raimondas Sasnauskas <sasnausk@informatik.uni-tuebingen.de>  */
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
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA  */
/*                                                                        */
/**************************************************************************/

#ifdef IDMEF_SUPPORT_ENABLED

#include "idmefmessage.h"

#include <string>
#include <iostream>
#include <sstream>

#include <sys/time.h>
#include <time.h>

IdmefMessage::IdmefMessage(const std::string& analyzerName, const std::string& analyzerId,
			   const std::string& classification, MessageType type) 
{
        this->analyzerName = analyzerName;
	this->analyzerId = analyzerId;
        this->classification = classification;
	if (type == ALERT) {
		this->messageType = "Alert";
		createAlertBody();
	} else if (type == HEARTBEAT) {
		this->messageType = "Heartbeat";
		createHeartbeatBody();
	} else {
		throw exceptions::XMLException("Unknown message type " + type);
	}
        init();
}

IdmefMessage::IdmefMessage()
{
        this->analyzerName = "unknown-module";
        this->classification = "unknown";
        createAlertBody();
        init();
}

IdmefMessage::~IdmefMessage()
{
        /* clean libxml library */
        xmlFreeDoc(idmefTree);
        xmlCleanupParser();
}

void IdmefMessage::init()
{
        /* parse the default IDMEF-Message */
        idmefTree = xmlReadMemory(idmefMessage.c_str(), idmefMessage.size(), "noname.xml", NULL, 0);
        if (idmefTree == NULL) {
                throw exceptions::XMLException("Error parsing the default IDMEF-message!");
        }
}

void IdmefMessage::createAlertBody()
{
        /* create the default IDMEF-Message <Alert> body */
        idmefMessage =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
//                 "<!DOCTYPE IDMEF-Message PUBLIC \"-//IETF//DTD RFC XXXX IDMEF v1.0//EN\" \"idmef-message.dtd\" \n"
//                 "[<!ENTITY % x-diadem SYSTEM \"diadem.dtd\"> %x-diadem; ]>\n"
//                 "<IDMEF-Message version=\"1.0\" xmlns=\"http://iana.org/idmef\">\n"
                "<IDMEF-Message version=\"1.0\">\n"
                "<Alert messageid=\"0\">\n"
                "<Analyzer name=\"" + analyzerName + "\" analyzerid=\"" + analyzerId + "\" "
		"manufacturer=\"\" model=\"\" version=\"\" class=\"\" ostype=\"\" osversion=\"\"/>\n"
                "<CreateTime ntpstamp=\"\"/>\n"
                "<Source/>\n"
                "<Target/>\n"
                "<Classification text=\"" + classification  + "\"></Classification>\n"
                "<Assessment/>\n"
                "<AdditionalData type=\"xml\"/>\n"
                "</Alert>\n"
                "</IDMEF-Message>";

        /* register multiple nodes */
        multipleNodes.push_back("Address");
        multipleNodes.push_back("DIADEM:ObservationPoint");
	multipleNodes.push_back("Service");

}

void IdmefMessage::createHeartbeatBody()
{
        /* create the default IDMEF-Message <Heartbeat> body */
        idmefMessage =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
//                 "<!DOCTYPE IDMEF-Message PUBLIC \"-//IETF//DTD RFC XXXX IDMEF v1.0//EN\" \"idmef-message.dtd\" \n"
//                 "[<!ENTITY % x-diadem SYSTEM \"diadem.dtd\"> %x-diadem; ]>\n"
//                 "<IDMEF-Message version=\"1.0\" xmlns=\"http://iana.org/idmef\">\n"
                "<IDMEF-Message version=\"1.0\">\n"
                "<Heartbeat messageid=\"0\">\n"
                "<Analyzer name=\"" + analyzerName + "\" analyzerid=\"" + analyzerId + "\" "
		"manufacturer=\"\" model=\"\" version=\"\" class=\"\" ostype=\"\" osversion=\"\"/>\n"
                "<CreateTime ntpstamp=\"\"/>\n"
		"<HeartbeatInterval/>\n"
		"<AnalyzerTime ntpstamp=\"\"/>\n"
		"<AdditionalData type=\"xml\"/>\n"
		"</Heartbeat>\n"
                "</IDMEF-Message>";
}

void IdmefMessage::setMessageId(int id)
{
        /* TODO: how to calculate message id? */
        std::ostringstream oss;
        oss << id; 
        std::string messageId = analyzerName + "-" + oss.str();
        setIdmefNodeAttr("IDMEF-Message", messageType, "messageid", messageId);
}

void IdmefMessage::createAnalyzerNode(const std::string& category, const std::string& address,
                                      const std::string& netmask, const std::string& location) 
{
        xmlNodePtr currNode = createIdmefNode("Analyzer", "Node", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating <Node> node failed" << std::endl;
                return;
        }
        //setIdmefNodeAttr(currNode, "Node", "category", "unknown");

        if (location != "NULL")
                createIdmefNode(currNode, "location", location);

        currNode = createIdmefNode(currNode, "Address", "NULL");

        if (category == "ipv4-addr" || category == "ipv6-addr")
                setIdmefNodeAttr(currNode, "Address", "category", category);
        else
                setIdmefNodeAttr(currNode, "Address", "category", "unknown");

        createIdmefNode(currNode, "address", address);
        createIdmefNode(currNode, "netmask", netmask);
}

void IdmefMessage::createAnalyzerNode(const std::string& name, const std::string& location)
{
        xmlNodePtr currNode = createIdmefNode("Analyzer", "Node", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating <Node> node failed" << std::endl;
                return;
        }
        setIdmefNodeAttr(currNode, "Node", "category", "unknown");

        createIdmefNode(currNode, "name", name);
        
        if (location != "NULL")
                createIdmefNode(currNode, "location", location);
}

void IdmefMessage::setAnalyzerAttr(const std::string& analyzerClass, const std::string& manufacturer,
                                   const std::string& model, const std::string& version)
{
        setIdmefNodeAttr(messageType, "Analyzer", "class", analyzerClass);
        setIdmefNodeAttr(messageType, "Analyzer", "manufacturer", manufacturer);
        setIdmefNodeAttr(messageType, "Analyzer", "model", model);
        setIdmefNodeAttr(messageType, "Analyzer", "version", version); 
}

void IdmefMessage::setAnalyzerNodeIdAttr(const std::string& ident)
{
        setIdmefNodeAttr("Analyzer", "Node", "ident", ident);
}

void IdmefMessage::createCreateTimeNode()
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "CreateTime");
        setIdmefNodeContent(currNode, getLocalTime());
	setIdmefNodeAttr(currNode, "CreateTime", "ntpstamp", getNtpStamp());
}

void IdmefMessage::createSourceNode(const std::string& spoofed, const std::string& category,
                                    const std::string& address, const std::string& netmask)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "Source");
        setIdmefNodeAttr(currNode, "Source", "spoofed", spoofed);

        currNode = createIdmefNode(currNode, "Node", "NULL");
        setIdmefNodeAttr(currNode, "Node", "category", "unknown");

        currNode = createIdmefNode(currNode, "Address", "NULL");
        setIdmefNodeAttr(currNode, "Address", "category", category);

        createIdmefNode(currNode, "address", address);
        createIdmefNode(currNode, "netmask", netmask);
}

void IdmefMessage::createTargetNode(const std::string& decoy, const std::string& category,
                                    const std::string& address, const std::string& netmask)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "Target");
        setIdmefNodeAttr(currNode, "Target", "decoy", decoy);

        currNode = createIdmefNode(currNode, "Node", "NULL");
        setIdmefNodeAttr(currNode, "Node", "category", "unknown");

        currNode = createIdmefNode(currNode, "Address", "NULL");
        setIdmefNodeAttr(currNode, "Address", "category", category);

        createIdmefNode(currNode, "address", address);
	createIdmefNode(currNode, "netmask", netmask);
}

void IdmefMessage::createServiceNode(const std::string& nodeName, const std::string& name, const std::string& port,
				     const std::string& portlist, const std::string& protocol)
{
	xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, nodeName);
	if (currNode == NULL) {
                std::cerr << "Error: creating <Service> node failed. Unknown node name \"" << nodeName << "\"" << std::endl;
                return;
        }

	currNode = createIdmefNode(currNode, "Service", "NULL");
	setIdmefNodeAttr(currNode, "Service", "ident", "0");
	setIdmefNodeAttr(currNode, "Service", "ip_version", "4");
	setIdmefNodeAttr(currNode, "Service", "iana_protocol_number", "");
	setIdmefNodeAttr(currNode, "Service", "iana_protocol_name", "");

	if (name != "") {
		createIdmefNode(currNode, "name", name);
	}
	if (port != "") {
		createIdmefNode(currNode, "port", port);
	}
	if (portlist != "") {
		createIdmefNode(currNode, "portlist", portlist);
	}
	if (protocol != "") {
		createIdmefNode(currNode, "protocol", protocol);
	}
}

void IdmefMessage::setServiceNodeAttr(const std::string& nodeName, const std::string& ident, 
				      const std::string& ip_version, const std::string& iana_protocol_number, 
				      const std::string& iana_protocol_name)
{
	xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, nodeName);
	if (currNode == NULL) {
                std::cerr << "Error: Can't find node name \"" << nodeName << "\"" << std::endl;
                return;
        }

	currNode = findIdmefNode(currNode, "Service");
	if (currNode == NULL) {
                std::cerr << "Error: Can't find <Service> node" << std::endl;
                return;
        }

	if (ident != "")
		setIdmefNodeAttr(currNode, "Service", "ident", ident);
	if (ip_version != "")
		setIdmefNodeAttr(currNode, "Service", "ip_version", ip_version);
	if (iana_protocol_number != "")
		setIdmefNodeAttr(currNode, "Service", "iana_protocol_number", iana_protocol_number);
	if (iana_protocol_name != "")
		setIdmefNodeAttr(currNode, "Service", "iana_protocol_name", iana_protocol_name);
}

void IdmefMessage::createAssessmentNode(const std::string& impactSeverity, const std::string& impactType,
                                       const std::string& impact, const std::string& confidence)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "Assessment");

        createIdmefNode(currNode, "Impact", impact);
        setIdmefNodeAttr(currNode, "Impact", "severity", impactSeverity);
        setIdmefNodeAttr(currNode, "Impact", "type", impactType);

        createIdmefNode(currNode, "Confidence", confidence);
}

void IdmefMessage::createExtAssessmentNode(const std::string& severity, const std::string& spread,
                                           const std::string& intensity, const std::string& confidence)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "DIADEM:Observation");
        if (currNode == NULL)
                currNode = createIdmefNode("AdditionalData", "DIADEM:Observation", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating observation node failed" << std::endl;
                return;
        }
                
        currNode = createIdmefNode(currNode, "DIADEM:Assessment", "NULL");

        createIdmefNode(currNode, "DIADEM:Severity", severity);
        setIdmefNodeAttr(currNode, "DIADEM:Severity", "Base", "1000");

        createIdmefNode(currNode, "DIADEM:Spread", spread);
        setIdmefNodeAttr(currNode, "DIADEM:Spread", "Base", "1000");

        createIdmefNode(currNode, "DIADEM:Intensity", intensity);
        setIdmefNodeAttr(currNode, "DIADEM:Intensity", "Base", "1000");

        createIdmefNode(currNode, "DIADEM:Confidence", confidence);
        setIdmefNodeAttr(currNode, "DIADEM:Confidence", "Base", "1000");
}

void IdmefMessage::setExtAssessmentNodeAttr(const std::string& assessmentNode, const std::string& base)
{
        setIdmefNodeAttr("DIADEM:Assessment", assessmentNode, "Base", base);
}

void IdmefMessage::createExtHttpParametersNode(const std::string& method, const std::string& uri,
					       const std::string& result)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "DIADEM:Observation");
        if (currNode == NULL)
                currNode = createIdmefNode("AdditionalData", "DIADEM:Observation", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating observation node failed" << std::endl;
                return;
        }

        currNode = createIdmefNode(currNode, "DIADEM:HTTP", "NULL");
	currNode = createIdmefNode(currNode, "DIADEM:Parameters", "NULL");

	createIdmefNode(currNode, "DIADEM:Method", method);
	createIdmefNode(currNode, "DIADEM:Uri", uri);
	createIdmefNode(currNode, "DIADEM:Result", result);
}

void IdmefMessage::createExtStatisticsNode(const std::string& octetCount, const std::string& packetCount, 
	const std::string& flowCount, const std::string& octetRate, const std::string& packetRate, 
	const std::string& anomalyMeasure)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "DIADEM:Observation");
        if (currNode == NULL)
                currNode = createIdmefNode("AdditionalData", "DIADEM:Observation", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating observation node failed" << std::endl;
                return;
        }

	currNode = findIdmefNode(currNode, "DIADEM:Statistics");
	if (currNode == NULL)
		currNode = createIdmefNode("DIADEM:Observation", "DIADEM:Statistics", "NULL");      

        createIdmefNode(currNode, "DIADEM:OctetCount", octetCount);
        createIdmefNode(currNode, "DIADEM:PacketCount", packetCount);
        createIdmefNode(currNode, "DIADEM:FlowCount", flowCount);

        createIdmefNode(currNode, "DIADEM:OctetRate", octetRate);
        setIdmefNodeAttr(currNode, "DIADEM:OctetRate", "exponent", "0");

        createIdmefNode(currNode, "DIADEM:PacketRate", packetRate);
        setIdmefNodeAttr(currNode, "DIADEM:PacketRate", "exponent", "0");

        createIdmefNode(currNode, "DIADEM:AnomalyMeasure", anomalyMeasure);
        setIdmefNodeAttr(currNode, "DIADEM:AnomalyMeasure", "Base", "1000");
}

void IdmefMessage::createExtHTTPStatisticsNode(const std::string& RequestRate, const std::string& RequestRateLCL,
					       const std::string& RequestRateUCL)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "DIADEM:Observation");
        if (currNode == NULL)
                currNode = createIdmefNode("AdditionalData", "DIADEM:Observation", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating observation node failed" << std::endl;
                return;
        }

	currNode = findIdmefNode(currNode, "DIADEM:Statistics");
	if (currNode == NULL)
		currNode = createIdmefNode("DIADEM:Observation", "DIADEM:Statistics", "NULL");      

	createIdmefNode(currNode, "DIADEM:HTTPRequestRate", RequestRate);
        setIdmefNodeAttr(currNode, "DIADEM:HTTPRequestRate", "exponent", "0");

        createIdmefNode(currNode, "DIADEM:HTTPRequestRateLCL", RequestRateLCL);
        setIdmefNodeAttr(currNode, "DIADEM:HTTPRequestRateLCL", "exponent", "0");
	
        createIdmefNode(currNode, "DIADEM:HTTPRequestRateUCL", RequestRateUCL);
        setIdmefNodeAttr(currNode, "DIADEM:HTTPRequestRateUCL", "exponent", "0");
}

void IdmefMessage::setExtStatisticsNodeAttr(const std::string& statisticsNode, const std::string& value)
{
        if (statisticsNode == "DIADEM:AnomalyMeasure")
                setIdmefNodeAttr("DIADEM:Statistics", statisticsNode, "Base", value);
        else if (statisticsNode == "DIADEM:OctetRate" 
		 || statisticsNode == "DIADEM:PacketRate"
		 || statisticsNode == "DIADEM:HTTPRequestRate" 
		 || statisticsNode == "DIADEM:HTTPRequestRateLCL"
		 || statisticsNode == "DIADEM:HTTPRequestRateUCL")
                setIdmefNodeAttr("DIADEM:Statistics", statisticsNode, "exponent", value);
}

void IdmefMessage::createExtObservationPointNode(const std::string& OPName, const std::string& IPv4Address)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "DIADEM:Observation");
        if (currNode == NULL)
                currNode = createIdmefNode("AdditionalData", "DIADEM:Observation", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating observation node failed" << std::endl;
                return;
        }

        currNode = createIdmefNode(currNode, "DIADEM:ObservationPoint", "NULL");

        createIdmefNode(currNode, "DIADEM:OPName", OPName);
        createIdmefNode(currNode, "DIADEM:IPv4Address", IPv4Address);
}

void IdmefMessage::createExtNetworkLayerNode()
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "DIADEM:Observation");
        if (currNode == NULL)
                currNode = createIdmefNode("AdditionalData", "DIADEM:Observation", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating observation node failed" << std::endl;
                return;
        }

        currNode = findIdmefNode(currNode, "DIADEM:TrafficDescription");
        if (currNode == NULL)
                currNode = createIdmefNode("DIADEM:Observation", "DIADEM:TrafficDescription", "NULL");
        if (currNode == NULL) {
                std::cerr << "Error: creating traffic description node failed" << std::endl;
                return;
        }

        createIdmefNode(currNode, "DIADEM:NetworkLayer", "NULL");
        setIdmefNodeAttr(currNode, "DIADEM:NetworkLayer", "protocol", "IPv4");
}

void IdmefMessage::createExtNetworkLayerAddrNode(const std::string& src_dst, const std::string& IPv4Address, 
                                                 const std::string& IPv4Netmask)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "DIADEM:NetworkLayer");
        if (currNode == NULL) {
                std::cerr << "Error: <NetworkLayer> node doesn't exist. Call createNetworkLayerNode() first" << std::endl;
                return;
        }

        if (src_dst == "src") {
                currNode = createIdmefNode(currNode, "DIADEM:IPv4Source", "NULL");
                createIdmefNode(currNode, "DIADEM:IPv4Address", IPv4Address);
                createIdmefNode(currNode, "DIADEM:IPv4Netmask", IPv4Netmask);
        } else if (src_dst == "dst") {
                currNode = createIdmefNode(currNode, "DIADEM:IPv4Destination", "NULL");
                createIdmefNode(currNode, "DIADEM:IPv4Address", IPv4Address);
                createIdmefNode(currNode, "DIADEM:IPv4Netmask", IPv4Netmask);
        }
        else
                std::cerr << "Error: target \"" + src_dst + "\" unknown" << std::endl; 
}

void IdmefMessage::createExtNetworkLayerElemNode(const std::string& nodeName, const std::string& value)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, "DIADEM:NetworkLayer");
        if (currNode == NULL) {
                std::cerr << "Error: NetworkLayer node doesn't exist. Call createNetworkLayerNode() first" << std::endl;
                return;
        }

        createIdmefNode(currNode, nodeName, value);
}

xmlNodePtr IdmefMessage::createIdmefNode(const std::string& parentName, const std::string& nodeName, 
                                         const std::string& nodeContent)
{
        xmlNodePtr currNode, newNode;
        currNode = xmlDocGetRootElement(idmefTree);

        if (currNode == NULL) {
                std::cerr << "empty document" << std::endl;
                return NULL;
        }
        if (findIdmefNode(currNode, nodeName) != NULL) {
                if (!isMultipleNode(nodeName)) {
                        std::cerr << "Error: node \"" << nodeName << "\" already exists" << std::endl; 
                        return NULL;
                }
        }
        currNode = findIdmefNode(currNode, parentName);
        if (currNode == NULL) {
                std::cerr << "Error: parent node \"" << parentName << "\" doesn't exist" << std::endl;
                return NULL;
        }
        if (nodeContent != "NULL") {
                newNode = xmlNewChild(currNode, NULL, (const xmlChar*) nodeName.c_str(), 
                              (const xmlChar*) nodeContent.c_str());
        }
        else
                newNode = xmlNewChild(currNode, NULL, (const xmlChar*) nodeName.c_str(), NULL);

        return newNode;
}

xmlNodePtr IdmefMessage::createIdmefNode(xmlNodePtr parentName, const std::string& nodeName,
                                         const std::string& nodeContent)
{
        xmlNodePtr newNode;
        if (findIdmefNode(parentName, nodeName) != NULL) {
                if (!isMultipleNode(nodeName)) {
                        std::cerr << "Error: node \"" << nodeName << "\" already exists" << std::endl;
                        return NULL;
                }
        }
        if (nodeContent != "NULL") {
                newNode = xmlNewChild(parentName, NULL, (const xmlChar*) nodeName.c_str(),
                                      (const xmlChar*) nodeContent.c_str());
        }
        else
                newNode = xmlNewChild(parentName, NULL, (const xmlChar*) nodeName.c_str(), NULL);

        return newNode;
}

void IdmefMessage::setIdmefNodeContent(const std::string& parentName, const std::string& nodeName,
                                       const std::string& content)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, parentName);
        if (currNode == NULL) {
                std::cerr << "Error: parent node \"" << parentName << "\" doesn't exist" << std::endl;
                return;
        }
        currNode = findIdmefNode(currNode, nodeName);
        if (currNode == NULL) {
                std::cerr << "Error: node \"" << nodeName << "\" doesn't exist" << std::endl;
                return;
        }
        
        xmlNodeSetContent(currNode, (xmlChar*) content.c_str());
}
 
void IdmefMessage::setIdmefNodeContent(xmlNodePtr parentName, const std::string& content)
{
        xmlNodeSetContent(parentName, (xmlChar*) content.c_str());
}

void IdmefMessage::setIdmefNodeAttr(const std::string& parentName, const std::string& nodeName, 
                                    const std::string& nodeAttr, const std::string& attrContent)
{
        xmlNodePtr currNode = xmlDocGetRootElement(idmefTree);
        currNode = findIdmefNode(currNode, parentName);
        if (currNode != NULL) {
                currNode = findIdmefNode(currNode, nodeName);   
                if (currNode != NULL) {
                        xmlSetProp(currNode, (const xmlChar*) nodeAttr.c_str(), 
                                   (const xmlChar*) attrContent.c_str());                       
                }
                else
                        std::cerr << "Error: node \"" << nodeName << "\" doesn't exist" << std::endl;
        }
        else    
                std::cerr << "Error: parent node \"" << parentName << "\" doesn't exist" << std::endl;
}

void IdmefMessage::setIdmefNodeAttr(xmlNodePtr parentName, const std::string& nodeName,
                                    const std::string& nodeAttr, const std::string& attrContent)
{
        xmlNodePtr currNode;
        if (parentName != NULL) {
                currNode = findIdmefNode(parentName, nodeName);
                if (currNode != NULL) {
                        xmlSetProp(currNode, (const xmlChar*) nodeAttr.c_str(),
                                   (const xmlChar*) attrContent.c_str());
                }
                else
                        std::cerr << "Error: node \"" << nodeName << "\" doesn't exist" << std::endl;
        }
        else
                std::cerr << "Error: there is no parent node for \"" << nodeName << "\" node." << std::endl;

}

xmlNodePtr IdmefMessage::findIdmefNode(xmlNodePtr nodeIter, const std::string& nodeName)
{
        while (nodeIter != NULL) {
                if (xmlStrEqual(nodeIter->name, (const xmlChar*) nodeName.c_str())) {
                        return nodeIter;
                }
                xmlNodePtr tmp = findIdmefNode(nodeIter->xmlChildrenNode, nodeName);
                if (tmp != NULL)
                        return tmp;
                nodeIter = nodeIter->next;
        }
        return NULL;
}

void IdmefMessage::publish(XmlBlasterCommObject& comm, const std::string& topic)
{
        /* set time stamp */
        createCreateTimeNode();
        xmlBufferPtr xmlBufPtr = xmlBufferCreate();
        int r = xmlNodeDump(xmlBufPtr, idmefTree, xmlDocGetRootElement(idmefTree), 0, 1);
        comm.publish ((char *) xmlBufPtr->content, topic);
        xmlBufferFree (xmlBufPtr);
}


#if 0
// this is for debugging purposes
void IdmefMessage::toString()
{
        /* set time stamp */
        createCreateTimeNode();

        /* print the IDMEF-Message */
        xmlIndentTreeOutput = 1;
        xmlKeepBlanksDefault(0);
        xmlBufferPtr xmlBufPtr = xmlBufferCreate();
        xmlNodeDump (xmlBufPtr, idmefTree, xmlDocGetRootElement(idmefTree), 0, 1);
        std::cout << std::string((char *)xmlBufPtr->content, (char *)xmlBufPtr->content + xmlBufPtr->use) 
		  << std::endl;
        xmlBufferFree(xmlBufPtr);
        xmlCleanupParser();
}
#endif

bool IdmefMessage::isMultipleNode(const std::string& nodeName)
{
        bool b = false;
        for (unsigned int i = 0; i < multipleNodes.size(); i++) {
                if (multipleNodes[i] == nodeName)
                        return true;
        }
        return b;
}

std::string IdmefMessage::getLocalTime()
{
        time_t rawtime = time(NULL);
        if (rawtime == ((time_t) - 1)) {
                std::cerr << "time failed" << std::endl;
                return "unknown";
        }

        /** 
         * Date and time format to ISO 8601:2000 standard
         * 2005-11-04T13:59:15+0100
         */
        struct tm* ptm;
        ptm = localtime(&rawtime);
        char tmp[30];
        strftime(tmp, 30, "%Y-%m-%dT%H:%M:%S%z", ptm);
        return std::string(tmp);
}

std::string IdmefMessage::getNtpStamp()
{
	/* NTP Timestamp, for example "0x12345678.0x87654321" */
	std::stringstream ss;
	timeval t;
	gettimeofday(&t, 0);
	ss << "0x" << t.tv_sec + OFFSET_1970 << ".0x" << (((uint64_t)t.tv_usec) << 32) / 1000000;  
	return ss.str();
}

#endif //IDMEF_SUPPORT_ENABLED
