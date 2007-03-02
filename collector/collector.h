/**************************************************************************/
/*    Copyright (C) 2005-2007 Lothar Braun <mail@lobraun.de>              */
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

#ifndef _COLLECTOR_H_
#define _COLLECTOR_H_

/**
 * @author Lothar Braun <braunl@informatik.uni-tuebingen.de>
 */

#include <concentrator/ipfix.h>
#include <concentrator/rcvIpfix.h>
#include <commonutils/metering.h>


#include <string>


class Manager;
class CollectorConfObj;
class RecorderBase;
class DetectModExporter;
class XMLConfObj;


/**
 * Collector reads IPFIX packets from the network and passes them to
 * a detectionmodule exporter object.
 * The detection module manager is started and controlled by this class 
 */
class Collector
{
public:
        /**
         * Constructor. Constructs the main collector object.
         */
        Collector();

        /**
         * Destructor. Deinitializes the mail collector object.
         */
        ~Collector();
        
        /**
         * Parses the configuration file.
         * @param config_file The configuration file.
         */
        void readConfig(const std::string& config_file);

        /**
         * Starts all detection modules specified in the configuration file (see @c read_config())
         */
        void startModules();

        /**
         * Main function.
         * This function starts the manager thread (see @c manager::run()).
         * After starting the manger, the functions waits for incoming IPFIX packets.
         */
        void run();

        /**
         * Sets the port the collector will listen on.
         * @param port Port number
         */
        void setListenPort(int port);

        /**
         * Sets collectors receiver type (TCP/IPv4, UDP/Ipv4, ...)
         * @param r_t Type of receiver
         */
        void setReceiverType(Receiver_Type r_t);

 private:
        static Manager* man;
        static bool terminateCollector;
	static RecorderBase* recorder;

        static DetectModExporter* exporter;
	static Metering* metering;
        static bool replaying;

	std::string packetDir;
	
	/**
	 * Read working dir from configuration file.
	 * @param confObj Configuration object
	 */
	void readWorkingDir(XMLConfObj* config);

	/**
	 * Read list of detection modules from configuration file.
	 * @param confObj Configuration object
	 */
	void readDetectionModules(XMLConfObj* config);

	/**
	 * Read miscellaneous items from the configuration file.
	 * @param confObj Configuration object
	 */
	void readMisc(XMLConfObj* config);

	/**
	 * Read all information necessary to exchange IPFIX packets
	 * between collector and detection modules.
	 * 
	 * There are currently two possible exchange methods:
	 * 1.) Via a file system (stable and good tested, but slow)
	 * 2.) Via a shared memory block (unstable and not so well tested, but very fast)
	 * @param confObj Configuration object
	 */
	void readExchangeProtocol(XMLConfObj* config);

	/**
	 * Read all information that is needed for recording or replaying traffic.
	 * @param confObj Configuration object
	 */
	void readRecording(XMLConfObj* config);

	/**
	 * Read IDMEF specific stuff if IDMEF-Support is enabled.
	 * @param confObj Configuration object
	 */
	void readIDMEF(XMLConfObj* config);

 protected:
        /**
         * Function replacing processMessage() in libconcentrator.
         * This function takes all the IPFIX packets coming from the network interfaces and writes them
         * to the path specified in the configuration file (see @c collector::read_config())
         * @param ifr not used
         * @param data Data extracted from the IPFIX packet
         * @param len Length of extracted data
         * @return 0 if operation succeded, -1 otherwise
         */
        static int messageCallBackFunction(IpfixParser*, byte* data, uint16_t len);

        /**
         * Signal handler for signal SIGINT.
         * The function initiates cleanup process.
         * @param sig Emitted signal
         */
        static void sigInt(int sig);


        int listenPort;
        Receiver_Type receiverType;
};

#endif
