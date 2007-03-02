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

#ifndef _GLOBAL_H_
#define _GLOBAL_H_


#include "exceptions.h"
#include "../concentrator/rcvIpfix.h"


#include <stdarg.h>


#include <string>
#include <ostream>


namespace config_space 
{
        static const std::string path_to_config = "./";
        static const std::string collector_config = "collector.xml";
        //static const std::string packet_path = "../packet_dir/";
                
        static const int SHM_SIZE = 100;
        static const int MAX_PATH_SIZE = 30;
        
        static const unsigned MAX_FILES = 100000;

        static const int DEFAULT_LISTEN_PORT = 4711;
        static const Receiver_Type DEFAULT_TRANSPORT_PROTO = UDP_IPV4;

        /* entries in xml-files */
	static const std::string CONFIGURATION="configuration";
	static const std::string WORKING_DIR="workingDir";
	static const std::string DETECTIONMODULES="modules";
	static const std::string DETECTIONMODULE="module";
	static const std::string FILENAME="filename";
	static const std::string CONFIG_FILE="configFile";
	static const std::string RUN="run";
	static const std::string ARG="arg";
        static const std::string COLLECTOR_STRING="collector";
        static const std::string LISTEN_PORT="listenPort";
        static const std::string KILL_TIME="detectmod_killtime";
        static const std::string RESTART_ON_CRASH="restartOnCrash";
        static const std::string PACKET_DIRECTORY="packetDir";
	static const std::string PLAYER="player";
	static const std::string EXCHANGE_PROTOCOL="exchangeProtocol";
	static const std::string EP_TYPE="type";
	static const std::string EP_FILES="files";
	static const std::string EP_SHM="shm";
	static const std::string SHMSIZE="shmSize";
	static const std::string TRAFFIC_DIR="trafficDir";
	static const std::string ACTION="action";
	static const std::string RECORD="record";
	static const std::string REPLAY="replay";
	static const std::string OFF="off";
	static const std::string TOPAS="topas";
	static const std::string XMLBLASTERS="xmlBlasters";
	static const std::string XMLBLASTER="xmlBlaster";
	static const std::string XMLBLASTER_PROP="prop";
	static const std::string MANAGER_ID="managerID";
	static const std::string DEFAULT_MANAGER_ID="topas-manager";
	static const std::string START="start";
	static const std::string STOP="stop";
	static const std::string RESTART="restart";
	static const std::string GET_MODULE_CONFIG="getModuleConfig";
	static const std::string GET_AVAILABLE_MODULES="getAvailableModules";
	static const std::string GET_RUNNING_MODULES="getRunningModules";
	static const std::string UPDATE_MODULE_CONFIG="updateModuleConfig";
	static const std::string MODULE_FILENAME="fileName";

        static const int MAX_IPFIX_PACKET_LENGTH=65536;
        static const unsigned DEFAULT_KILL_TIME = 30;
};

namespace error_states 
{
        static const int INIT_ERROR = -1;
        static const int CONFIG_ERROR = -2;
        static const int DETECTION_MODULE_ERROR = -3;
        static const int RUN_ERROR = -4;
}

#endif
