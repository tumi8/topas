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

#ifndef IDMEFMESSAGE_H
#define IDMEFMESSAGE_H

/**
 * @author Raimondas Sasnauskas <sasnausk@informatik.uni-tuebingen.de>
 * @author Lothar Braun <braunl@informatik.uni-tuebingen.de>
 */

#include <libxml/encoding.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <string>
#include <vector>

#include "xmlBlasterCommObject.h"

/**
 * The IdmefMessage class creates a IDMEF-Message with the DIADEM extension
 * and publishes it to the xmlBlaster
 */
class IdmefMessage {
public:
        /**
         * Default constructors.
         * @param analyzerName Name of the detection module.
         * @param classification Classification of the detection method.
         */
        IdmefMessage(const std::string& analyzerName, const std::string& classification);
        IdmefMessage();

        ~IdmefMessage();

        /**
         * Set IDMEF-Message alert ID.
         * @param id Message ID.
         */
        void setMessageId(int id);

        /**
         * Create analyzer description node.
         * @param category IP protocol version ("ipv4-addr" | "ipv6-addr").
         * @param address IP address of the node.
         * @param netmask Network mask.
         * @param location Location of the node (optional). Use "NULL" to suppress.
         */
        void createAnalyzerNode(const std::string& category, const std::string& address,
                                const std::string& netmask, const std::string& location);
        /**
         * Create analyzer description node.
         * @param name Name of the node.
         * @param location Location of the node (optional). Use "NULL" to suppress.
         */
        void createAnalyzerNode(const std::string& name, const std::string& location);

        /**
         * Set <Analyzer> node attributes (optional).
         * @param analyzerClass Analyzer class, e.g. "anomaly detection".
         * @param manufacturer Manufacturer, e.g. "DIADEM".
         * @param model Model, e.g. "statistical anomaly detection".
         * @param version Version number.
         */
        void setAnalyzerAttr(const std::string& analyzerClass, const std::string& manufacturer,
                             const std::string& model, const std::string& version);

        /**
         * Create <Source> node elements.
         * @param spoofed Optional. An indication of whether the source is, 
	 * as far as the analyzer can determine, a decoy ("unknown" | "yes" | "no").
         * @param category IP protocol version ("ipv4-addr" | "ipv6-addr").
         * @param address IP address of the node.
         * @param netmask Network mask.
         */
        void createSourceNode(const std::string& spoofed, const std::string& category,
                              const std::string& address, const std::string& netmask);

        /**
         * Create <Target> node elements.
         * @param decoy Optional. An indication of whether the target is,
	 * as far as the analyzer can determine, a decoy ("unknown" | "yes" | "no")
         * @param category IP protocol version ("ipv4-addr" | "ipv6-addr").
         * @param address IP address of the node.
         * @param netmask Network mask.
         */
        void createTargetNode(const std::string& decoy, const std::string& category,
                              const std::string& address, const std::string& netmask);

	/**
	 * Create <Service> node.
	 * @param nodeName The name of the element within the <Service> node is created ("Source" | "Target").
	 * @param name The name of the service.
	 * @param port The port number being used.
	 * @param portlist Zero or one. A list of port numbers being used.
	 * @param protocol Additional information about the protocol being used.
	 */
	void createServiceNode(const std::string& nodeName, const std::string& name, const std::string& port,
			       const std::string& portlist, const std::string& protocol);


	/**
	 * Set <Service> node attributes.
	 * @param nodeName The name of the element within the <Service> node is created ("Source" | "Target").
	 * @param ident Optional. A unique identifier for the service.
	 * @param ip_version Optional. The IP version number.
	 * @param iana_protocol_number Optional. The IANA protocol number.
	 * @param iana_protocol_name Optional. The IANA protocol name.
	 */
	void setServiceNodeAttr(const std::string& nodeName, const std::string& ident, const std::string& ip_version, 
				const std::string& iana_protocol_number, const std::string& iana_protocol_name);

        /**
         * Create <Assessment> node elements.
         * @param impactSeverity Impact severity description.
         * @param impactType Type of the impact.
         * @param impact Impact value.
         * @param confidence Confidence value.
         */
        void createAssessmentNode(const std::string& impactSeverity, const std::string& impactType,
                                  const std::string& impact, const std::string& confidence);

        /**
         * Create <DIADEM:Assessment> node elements.
         * @param Severity Severity of the attack.
         * @param Spread Spread of the attack.
         * @param Intensity Intensity of the attack.
         * @param Confidence Confidence of the attack.
         */
        void createExtAssessmentNode(const std::string& severity, const std::string& spread,
                                     const std::string& intensity, const std::string& confidence);

        /**
         * Set <DIADEM:Assessment> node elements attribute (optional).
         * @param assessmentNode Name of the node (Severity | Spread | Intensity | Confidence).
         * @param base Base of the value (default: 1000).
         */
        void setExtAssessmentNodeAttr(const std::string& assessmentNode, const std::string& base);

	/**
	 * Create <DIADEM:HTTP> node elements.
	 * @param method method used for attack.
	 * @param uri uri used for attack.
	 * @param result result used for attack.
	 */
	void createExtHttpParametersNode(const std::string& method, const std::string& uri,
					 const std::string& result);

        /**
         * Create <DIADEM:NetworkLayer> node.
         */
        void createExtNetworkLayerNode();

        /**
         * Create <DIADEM:IPv4Source> | <DIADEM:IPv4Destination> node.
         * @param src_dst Source | Destination (src | dst).
         * @param IPv4Address IPv4 address of the node.
         * @param IPv4Netmask Network mask.
         */
        void createExtNetworkLayerAddrNode(const std::string& src_dst, const std::string& IPv4Address, 
                                           const std::string& IPv4Netmask);

        /**
         * Create node representing IPv4 protocol header fields.
         */
        void createExtNetworkLayerElemNode(const std::string& nodeName, const std::string& value);

        /**
         * Create <DIADEM:Statistics> node elements.
         * @param OctetRate Volume in octets per second.
         * @param PacketRate Amount of packets per second.
         * @param PacketFrequency Packet frequency.
         * @param AnomalyMeasure Anomaly measure.
         */
        void createExtStatisticsNode(const std::string& octetRate, const std::string& packetRate,
                                     const std::string& packetFrequency, const std::string& anomalyMeasure);

	/**
	 * Create <DIADEM:Statistics> node elements for HTTP requests.
	 * @param RequestRate
	 * @param RequestRateLCL
	 * @param RequestRateUCL
	 */
	void createExtHTTPStatisticsNode(const std::string& RequestRate, const std::string& RequestRateLCL,
					 const std::string& RequestRateUCL);

        /**
         * Set <DIADEM:Statistics> node elements attribute (optional).
         * @param statisticsNode Node name (OctetRate | PacketRate | AnomalyMeasure).
         * @param value Value of the attribute.
         */
        void setExtStatisticsNodeAttr(const std::string& statisticsNode, const std::string& value);

        /**
         * Create <DIADEM:ObservationPoint> node elements.
         * @param OPName Name of the observation point.
         * @param IPv4Address IP address of the observation point.
         */
        void createExtObservationPointNode(const std::string& OPName, const std::string& IPv4Address);

        /* Print the IDMEF-Message to the standrard output */
        void toString();        

        /**
         * Publish the IDMEF-Message to the xmlBlaster.
         * @param comm xmlBlaster communication object.
         * @param topic Publish the IDMEF-Message under given topic.
         */
        void publish(XmlBlasterCommObject& comm, const std::string& topic);

private:
        /**
         * IDMEF-Message string
         */
        std::string idmefMessage;

        /**
         * Variables for the <Analyzer> and <Classification> nodes
         */
        std::string analyzerName;
        std::string classification;

        /**
         * Nodes, which can appear several times
         */
        std::vector<std::string> multipleNodes;

        /**
         * Pointer to the root of the IDMEF-Message
         */
        xmlDocPtr idmefTree;

        /**
         * Current date for <CreateTime> node
         */
        std::string getLocalTime();

        /**
         * Parse the default IDMEF-Message
         */
        void initXmlParser();

        /**
         * Create the default IDMEF-Message body
         */
        void createIdmefBody();

        /**
         * Create a single IDMEF node.
         * @param parentName Parent node name or pointer to it.
         * @param nodeName Node name.
         * @param nodeContent Node content (optional). Use "NULL" to suppress.
         * @return xmlNodePtr Pointer to the created node.
         */
        xmlNodePtr createIdmefNode(const std::string& parentName, const std::string& nodeName, 
                                   const std::string& nodeContent);

        xmlNodePtr createIdmefNode(xmlNodePtr parentName, const std::string& nodeName,
                                   const std::string& nodeContent);

        /**
         * Create <CreateTime> node.
         * Set the time stamp for the IDMEF-Message.
         */
        void createCreateTimeNode();

        /**
         * Find IDMEF node (recursive).
         * @param nodeIter Pointer to the starting node.
         * @param nodeName Node name to search for.
         * @return If nodeName is found, return a pointer to it, otherwise return NULL
         */
        xmlNodePtr findIdmefNode(xmlNodePtr nodeIter, const std::string& nodeName);

        /**
         * Set IDMEF node attribute.
         * @param parentName Parent node name or pointer to it.
         * @param nodeName Node name.
         * @param nodeAttr Attribute name.
         * @param attrContent Attribute content.
         */
        void setIdmefNodeAttr(const std::string& parentName, const std::string& nodeName, 
                              const std::string& nodeAttr, const std::string& attrContent);

        void setIdmefNodeAttr(xmlNodePtr parentName, const std::string& nodeName,
                              const std::string& nodeAttr, const std::string& attrContent);

        /**
         * Set IDMEF node value.
         * @param parentName Parent node name or pointer to it.
         * @param nodeName Node name.
         * @param content New content.
         */
        void setIdmefNodeContent(const std::string& parentName, const std::string& nodeName,
                                 const std::string& content);

        void setIdmefNodeContent(xmlNodePtr parentName, const std::string& content);

        /**
         * Checks if the node can appear several times.
         * @param nodeName Name of the node.
         * @return True or false.
         */
        bool isMultipleNode(const std::string& nodeName);
        
};

#endif

#endif //IDMEF_SUPPORT_ENABLED
