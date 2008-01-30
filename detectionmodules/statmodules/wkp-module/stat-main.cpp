/**************************************************************************/
/*    Copyright (C) 2006-07                                               */
/*    Romain Michalec, Sven Wiebusch, Gerhard Muenz                       */
/*    University of Tuebingen, Germany                                    */
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

// for pca (eigenvectors etc.)
#include <gsl/gsl_eigen.h>
// for pca (matrix multiplication)
#include <gsl/gsl_blas.h>

// for directory functions (mkdir, chdir ...)
#ifdef __unix__
#include <sys/types.h>
#include <sys/stat.h>
#elif __WIN32__ || _MS_DOS_
#include <dir.h>
#endif

#include<signal.h>
#include<math.h>

#include "stat-main.h"
#include "wmw-test.h"
#include "ks-test.h"
#include "pcs-test.h"
#include "cusum-test.h"

// ==================== Config File Constants =====================
#define CONFIGTAG_Preferences "preferences"
#define CONFIGTAG_OutputVerbosity "warning_verbosity"
#define CONFIGTAG_LogVerbosity "logfile_output_verbosity"
#define CONFIGTAG_LogFile "logfile"
#define CONFIGTAG_AlarmTime "alarm_time"
#define CONFIGTAG_SourceIds "accepted_source_ids"
#define CONFIGTAG_OfflineFile "offline_file"
#define CONFIGTAG_EndpointKey "endpoint_key"
#define CONFIGTAG_EndpointKeyProtocol "protocol"
#define CONFIGTAG_EndpointKeyPort "port"
#define CONFIGTAG_EndpointKeyAddress "address"
#define CONFIGTAG_Netmask "netmask"
#define CONFIGTAG_PCA "use_pca"
#define CONFIGTAG_CorrelationMatrix "use_correlation_matrix"
#define CONFIGTAG_PCALearningPhase "pca_learning_phase"
#define CONFIGTAG_Metrics "metrics"  // metric names are defined in mInfos array (see stat-main.cpp)
#define CONFIGTAG_QDFactor "qd_factor"
#define CONFIGTAG_Absolute "absolute"
#define CONFIGTAG_Negate "negate"
#define CONFIGTAG_OutputDir "output_dir"
#define CONFIGTAG_NoiseThresholdPackets "noise_threshold_packets"
#define CONFIGTAG_NoiseThresholdBytes "noise_threshold_bytes"
#define CONFIGTAG_MaxEndpoints "endpointlist_maxsize"
#define CONFIGTAG_TestFrequency "test_frequency"
#define CONFIGTAG_ReportOnlyFirstAttack "report_only_first_attack"
#define CONFIGTAG_PauseUpdateWhenAttack "pause_update_when_attack"
#define CONFIGTAG_XFrequentEndpoints "x_frequent_endpoints"
#define CONFIGTAG_EndpointsToMonitor "endpoints_to_monitor"
#define CONFIGTAG_CusumParams "cusum_parameters"
#define CONFIGTAG_CusumTest "cusum_test"
#define CONFIGTAG_AmplitudePercentage "amplitude_percentage"
#define CONFIGTAG_LogVarianceEWMA "log_variance_ewma"
#define CONFIGTAG_CusumLearningPhase "cusum_learning_phase"
#define CONFIGTAG_RepetitionFactor "repetition_factor"
#define CONFIGTAG_SmoothingConstant "smoothing_constant"
#define CONFIGTAG_WkpParams "wkp_parameters"
#define CONFIGTAG_WMWTest "wmw_test"
#define CONFIGTAG_KSTest "ks_test"
#define CONFIGTAG_PCSTest "pcs_test"
#define CONFIGTAG_OldSampleSize "old_sample_size"
#define CONFIGTAG_NewSampleSize "new_sample_size"
#define CONFIGTAG_WMWOneSided "wmw_one_sided"
#define CONFIGTAG_SignificanceLevel "significance_level"

// ==================== Default Constants =====================
#define DEFAULT_AlarmTime 10
#define DEFAULT_OutputVerbosity MsgStream::WARN
#define DEFAULT_LogFile "wkp_log.txt"
#define DEFAULT_LogVerbosity MsgStream::WARN
#define DEFAULT_NoiseThresholdPackets 0
#define DEFAULT_NoiseThresholdBytes 0
#define DEFAULT_MaxEndpoints 500
#define DEFAULT_TestFrequency 1
#define DEFAULT_XFrequentEndpoints 10
#define DEFAULT_AmplitudePercentage 3
#define DEFAULT_CusumLearningPhase 10
#define DEFAULT_RepetitionFactor 2
#define DEFAULT_SmoothingConstant 0.15
#define DEFAULT_OldSampleSize 30
#define DEFAULT_NewSampleSize 20
#define DEFAULT_SignificanceLevel 0.01
#define DEFAULT_PCALearningPhase 50

// ==================== Metric Info =====================
struct MetricInfo {
    const char* name; // metric name
    bool requiresBytes;
    bool requiresPackets;
};

struct MetricInfo mInfos[] = {
    /* mInfos[BYTES_IN] = */ {"bytes_in", true, false},
    /* mInfos[BYTES_OUT] = */ {"bytes_out", true, false},
    /* mInfos[PACKETS_IN] = */ {"packets_in", false, true},
    /* mInfos[PACKETS_OUT] = */ {"packets_out", false, true},
    /* mInfos[RECORDS_IN] = */ {"records_in", false, false},
    /* mInfos[RECORDS_OUT] = */ {"records_out", false, false},
    /* mInfos[BYTES_IN_PER_PACKET_IN] = */ {"bytes_per_packet_in", true, true},
    /* mInfos[BYTES_OUT_PER_PACKET_OUT] = */ {"bytes_per_packet_out", true, true},
    /* mInfos[BYTES_IN_PER_RECORD_IN] = */ {"bytes_per_record_in", true, false},
    /* mInfos[BYTES_OUT_PER_RECORD_OUT] = */ {"bytes_per_record_out", true, false},
    /* mInfos[PACKETS_IN_PER_RECORD_IN] = */ {"packets_per_record_in", false, true},
    /* mInfos[PACKETS_OUT_PER_RECORD_OUT] = */ {"packets_per_record_out", false, true},
    /* mInfos[BYTES_OUT_MINUS_BYTES_IN] = */ {"bytes_out-bytes_in", true, false},
    /* mInfos[PACKETS_OUT_MINUS_PACKETS_IN] = */ {"packets_out-packets_in", false, true},
    /* mInfos[RECORDS_OUT_MINUS_RECORDS_IN] = */ {"records_out-records_in", false, false},
    /* mInfos[BYTES_IN_MINUS_BYTES_OUT] = */ {"bytes_in-bytes_out", true, false},
    /* mInfos[PACKETS_IN_MINUS_PACKETS_OUT] = */ {"packets_in-packets_out", false, true},
    /* mInfos[RECORDS_IN_MINUS_RECORDS_OUT] = */ {"records_in-records_out", false, false},
};
    

// ==================== CONSTRUCTOR FOR CLASS Stat ====================


Stat::Stat(const std::string & configfile)
#ifdef OFFLINE_ENABLED
: DetectionBase<StatStore, OfflineInputPolicy<StatStore> >(configfile)
#else
: DetectionBase<StatStore>(configfile)
#endif
{
    msgStr.setName("wkp-/cusum-module");
    logStr.setName("wkp-/cusum-module");
    logStr.setLevel(MsgStream::NONE);

    // signal handlers
    if (signal(SIGTERM, sigTerm) == SIG_ERR) {
	msgStr.print(MsgStream::ERROR, "wkp-module: Couldn't install signal handler for SIGTERM.");
    }
    if (signal(SIGINT, sigInt) == SIG_ERR) {
	msgStr.print(MsgStream::ERROR, "wkp-module: Couldn't install signal handler for SIGINT.");
    }

    // lock, will be unlocked at the end of init() (cf. StatStore class header):
    StatStore::beginMonitoring = false;

    test_counter = 0;

    // parameter initialization
    init(configfile);

#ifdef OFFLINE_ENABLED
    /* open file with offline data */
    if(!OfflineInputPolicy<StatStore>::openOfflineFile(offlineFile.c_str())) {
	msgStr.print(MsgStream::FATAL, "Could not open offline data file!");
	stop();
    }
#else
    if(offlineFile != "")
	/* open file to store data for offline use */
	storefile.open(offlineFile.c_str());
#endif

}


Stat::~Stat() 
{

    if(storefile.is_open())
	storefile.close();

}


// =========================== init FUNCTION ==========================


// init()'s job is to extract user's preferences
// and test parameters from the XML config file
//
void Stat::init(const std::string & configfile) {

    XMLConfObj * config;
    config = new XMLConfObj(configfile, XMLConfObj::XML_FILE);

#ifdef IDMEF_SUPPORT_ENABLED
    /* register module */
    registerModule("stat-module");
#endif

    //
    // preferences section
    // 
    if (!config->nodeExists(CONFIGTAG_Preferences)) {
	msgStr.print(MsgStream::FATAL, "No preferences node defined in XML config file! Exiting.");
	delete config;
	stop();
    }

    config->enterNode(CONFIGTAG_Preferences);


    // extracting output verbosity
    if(config->nodeExists(CONFIGTAG_OutputVerbosity) && !(config->getValue(CONFIGTAG_OutputVerbosity)).empty()) 
    {
	switch (atoi(config->getValue(CONFIGTAG_OutputVerbosity).c_str())) {
	    case 0:
		msgStr.setLevel(MsgStream::NONE); break;
	    case 1:
		msgStr.setLevel(MsgStream::FATAL); break;
	    case 2:
		msgStr.setLevel(MsgStream::ERROR); break;
	    case 3:
		msgStr.setLevel(MsgStream::WARN); break;
	    case 4:
		msgStr.setLevel(MsgStream::INFO); break;
	    case 5:
		msgStr.setLevel(MsgStream::DEBUG); break;
	    default:
		msgStr << MsgStream::ERROR << "Invalid " << CONFIGTAG_OutputVerbosity 
		    << " parameter. Options are: 0=none, 1=fatal, 2=errors, 3=warnings, 4=info, 5=debug" << MsgStream::endl;
		stop();
	}
    } else {
	msgStr.setLevel(DEFAULT_OutputVerbosity);
    }
    msgStr << MsgStream::INFO << "Output verbosity is " << (unsigned) msgStr.getLevel() << " [0=none, 1=fatal, 2=errors, 3=warnings, 4=info, 5=debug]" << MsgStream::endl;

    // extracting logfile name
    std::string logfilename(DEFAULT_LogFile);
    if(config->nodeExists(CONFIGTAG_LogFile) && !(config->getValue(CONFIGTAG_LogFile)).empty())
	logfilename = config->getValue(CONFIGTAG_LogFile);
    if(!logStr.openLogfile(logfilename.c_str())) {
	msgStr.print(MsgStream::FATAL, "Could not open logfile! Check if you have enough rights to create or write to it.");
	stop();
    }
    msgStr.print(MsgStream::INFO, "Logging to file " + logfilename);

    // extracting logging verbosity
    if(config->nodeExists(CONFIGTAG_LogVerbosity) && !(config->getValue(CONFIGTAG_LogVerbosity)).empty()) 
    {
	switch (atoi(config->getValue(CONFIGTAG_LogVerbosity).c_str())) {
	    case 0:
		logStr.setLogLevel(MsgStream::NONE); break;
	    case 1:
		logStr.setLogLevel(MsgStream::FATAL); break;
	    case 2:
		logStr.setLogLevel(MsgStream::ERROR); break;
	    case 3:
		logStr.setLogLevel(MsgStream::WARN); break;
	    case 4:
		logStr.setLogLevel(MsgStream::INFO); break;
	    case 5:
		logStr.setLogLevel(MsgStream::DEBUG); break;
	    default:
		msgStr << MsgStream::ERROR << "Invalid " << CONFIGTAG_LogVerbosity 
		    << " parameter. Options are: 0=none, 1=p-values/attacks, 2+=cosmetics, 3+=learning/update/empty records, 4+=samples, 5=all" << MsgStream::endl;
		stop();
	}
    } else {
	logStr.setLogLevel(DEFAULT_LogVerbosity);
    }
    msgStr << MsgStream::INFO << "Logging verbosity is " << (unsigned) logStr.getLevel() << " [0=none, 1=p-values/attacks, 2+=cosmetics, 3+=learning/update/empty records, 4+=samples, 5=all]" << MsgStream::endl;

#ifndef OFFLINE_ENABLED
    // extracting source id's to accept
    if (config->nodeExists(CONFIGTAG_SourceIds) && !(config->getValue(CONFIGTAG_SourceIds)).empty()) {
	std::string str = config->getValue(CONFIGTAG_SourceIds);

	if (str != "all") {
	    msgStr << MsgStream::INFO << "Accepted source ids are: " << str << MsgStream::endl;
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
	} else {
	    msgStr << MsgStream::INFO << "All source ids are accepted" << MsgStream::endl;
	}
    } else {
	msgStr << MsgStream::INFO << "All source ids are accepted" << MsgStream::endl;
    }
#endif

    // extracting alarm_time
    // (that means that the test() methode will be called
    // atoi(alarm_time) seconds after the last test()-run ended)
#ifdef OFFLINE_ENABLED
    setAlarmTime(0);
    msgStr << MsgStream::INFO << "Alarm time is set to 0 during offline use." << MsgStream::endl;
#else
    if(config->nodeExists(CONFIGTAG_AlarmTime) && !(config->getValue(CONFIGTAG_AlarmTime)).empty()) {
	setAlarmTime( atoi(config->getValue(CONFIGTAG_AlarmTime).c_str()) );
	msgStr << MsgStream::INFO << "Alarm time is: " << atoi(config->getValue(CONFIGTAG_AlarmTime).c_str()) << MsgStream::endl;
    } else {
	msgStr << MsgStream::WARN << "No " << CONFIGTAG_AlarmTime << " parameter defined in XML config file! Using default: " << DEFAULT_AlarmTime << MsgStream::endl;
	setAlarmTime(DEFAULT_AlarmTime);
    }
#endif

    // extract filename for the file where
    // all the data will be written into
    if (config->nodeExists(CONFIGTAG_OfflineFile) && !(config->getValue(CONFIGTAG_OfflineFile)).empty()) {
	offlineFile = config->getValue(CONFIGTAG_OfflineFile);
	msgStr << MsgStream::INFO << "Offline file is: " << offlineFile << MsgStream::endl;
    } else {
#ifdef OFFLINE_ENABLED
	msgStr << MsgStream::FATAL << "No " << CONFIGTAG_OfflineFile << " parameter in XML config file! Exiting." << MsgStream::endl;
	stop();
#else
	offlineFile = "";
#endif
    }

    // extracting the netmask, which will be applied
    // to the ip of each endpoint; for aggregating
    // in ON- and OFFLINE MODE
    // NOTE: In OFFLINE MODE, only used, if endpoint_key contains "ip" or "all"
    if (config->nodeExists(CONFIGTAG_Netmask) && !(config->getValue(CONFIGTAG_Netmask)).empty()) {
	if (atoi(config->getValue(CONFIGTAG_Netmask).c_str()) >= 0 && atoi(config->getValue(CONFIGTAG_Netmask).c_str()) <= 32)
	    StatStore::netmask = atoi(config->getValue(CONFIGTAG_Netmask).c_str());
	else {
	    msgStr.print(MsgStream::FATAL, "Netmask must be between 0 and 32. Exiting.");
	    stop();
	}
    } else {
	StatStore::netmask = 32;
    }
    msgStr << MsgStream::INFO << "Netmask is set to " << StatStore::netmask << MsgStream::endl;

    // initialize pca parameters
    if (config->nodeExists(CONFIGTAG_PCA) && (config->getValue(CONFIGTAG_PCA) != "false")) {
	use_pca = true;

	if (config->nodeExists(CONFIGTAG_PCALearningPhase) && !(config->getValue(CONFIGTAG_PCALearningPhase)).empty())
	    pca_learning_phase = atoi(config->getValue(CONFIGTAG_PCALearningPhase).c_str());
	else
	    pca_learning_phase = DEFAULT_PCALearningPhase;

	if (config->nodeExists(CONFIGTAG_CorrelationMatrix) && (config->getValue(CONFIGTAG_CorrelationMatrix) != "false"))
	    use_correlation_matrix = true;
	else
	    use_correlation_matrix = false;

	msgStr << MsgStream::INFO << "PCA is used with " << (use_correlation_matrix ? "correlation" : "covariance") << " matrix and learning phase " << pca_learning_phase << MsgStream::endl;
    } else {
	use_pca = false;
    }

    // extracting metrics
    if ((config->selectNodeIfExists(CONFIGTAG_Metrics) && (config->enterNodeIfNotEmpty()))) {
	bool found;
	MetricData m;
	do {
	    std::stringstream tmp;
	    found = false;
	    for(int i=0; i < NUMBER_OF_METRICS; i++) {
		if(config->getNodeName() == mInfos[i].name) {
		    m.type = (Metric)i;
		    if(config->attributeExists(CONFIGTAG_QDFactor)) {
			m.applyQuasiDifference = true;
			m.factor = atof(config->getAttribute(CONFIGTAG_QDFactor).c_str());
		    } else {
			m.applyQuasiDifference = false;
			m.factor = 0.0;
		    }
		    if(config->attributeExists(CONFIGTAG_Absolute) && (config->getAttribute(CONFIGTAG_Absolute) != "false")) {
			m.absolute = true;
		    } else {
			m.absolute = false;
		    }
		    if(config->attributeExists(CONFIGTAG_Negate) && (config->getAttribute(CONFIGTAG_Negate) != "false")) {
			m.negate = true;
		    } else {
			m.negate = false;
		    }
		    // generate metric name
		    if (m.negate)
			tmp << "-";
		    if (m.absolute)
			tmp << "abs(";
		    tmp << mInfos[m.type].name;
		    if (m.applyQuasiDifference)
			tmp << "(t)-" << m.factor << mInfos[m.type].name << "(t-1)";
		    if (m.absolute)
			tmp << ")";
		    m.name = tmp.str();
		    metrics.push_back(m);
		    found = true;
#ifndef OFFLINE_ENABLED
		    if(mInfos[i].requiresBytes)
			subscribeTypeId(IPFIX_TYPEID_octetDeltaCount);
		    if(mInfos[i].requiresPackets)
			subscribeTypeId(IPFIX_TYPEID_packetDeltaCount);
#endif
		    break;
		}
	    }
	    if(!found)
		msgStr << MsgStream::ERROR << "Unknown metric \"" << config->getNodeName() << " in XML config file!" << MsgStream::endl;
	} while(config->selectNextNodeIfExists());
	config->leaveNode();
    } else {
	msgStr.print(MsgStream::WARN, "No metrics defined in XML config file!");
    }

    // print out, in which order the values will be stored
    // in the Samples-Lists (for better understanding the output
    // of test() etc.
    msgStr << MsgStream::INFO << "Tests will be performed on " << (use_pca?"PCs of metrics":"metrics") << " ordered as follows: ";
    for(std::vector<MetricData>::iterator val = metrics.begin(); val != metrics.end(); val++)
	msgStr << val->name << " ";
    msgStr << MsgStream::endl;

    // extract directory name for
    // output files of metrics and test-params
    createFiles = false;
    if (config->nodeExists(CONFIGTAG_OutputDir) && !(config->getValue(CONFIGTAG_OutputDir)).empty()) {
	output_dir = config->getValue(CONFIGTAG_OutputDir);
	if (mkdir(output_dir.c_str(), 0777) == -1) {
	    msgStr.print(MsgStream::FATAL, "Directory \"" + output_dir + "\" couldn't be created. It either already exists or you don't have enough rights. Exiting.");
	    stop();
	    return;
	} else {
	    msgStr.print(MsgStream::INFO, "Output directory is: " + output_dir);
	    createFiles = true;
	}
    }

    // extracting noise reduction preferences
    // extracting noise threshold for packets
    if(config->nodeExists(CONFIGTAG_NoiseThresholdPackets) && !(config->getValue(CONFIGTAG_NoiseThresholdPackets).empty()))
	noise_threshold_packets = atoi(config->getValue(CONFIGTAG_NoiseThresholdPackets).c_str());
    else
	noise_threshold_packets = DEFAULT_NoiseThresholdPackets;
    msgStr << MsgStream::INFO << "Noise threshold for packets is " << noise_threshold_packets << MsgStream::endl;
    // extracting noise threshold for bytes
    if(config->nodeExists(CONFIGTAG_NoiseThresholdBytes) && !(config->getValue(CONFIGTAG_NoiseThresholdBytes).empty()))
	noise_threshold_bytes = atoi(config->getValue(CONFIGTAG_NoiseThresholdBytes).c_str());
    else
	noise_threshold_bytes = DEFAULT_NoiseThresholdBytes;
    msgStr << MsgStream::INFO << "Noise threshold for bytes is " << noise_threshold_bytes << MsgStream::endl;

    // extracting endpoints to monitor
    init_endpoints(config);

    // extracting statistical test frequency preference
    if(config->nodeExists(CONFIGTAG_TestFrequency) && !(config->getValue(CONFIGTAG_TestFrequency).empty()))
	stat_test_frequency = atoi(config->getValue(CONFIGTAG_TestFrequency).c_str());
    else
	stat_test_frequency = DEFAULT_TestFrequency;
    msgStr << MsgStream::INFO << "Test frequency is " << stat_test_frequency << MsgStream::endl;

    // extracting report_only_first_attack parameter
    report_only_first_attack = true;
    if(config->nodeExists(CONFIGTAG_TestFrequency) && (config->getValue(CONFIGTAG_TestFrequency) == "false")) {
	report_only_first_attack = false;
	msgStr << MsgStream::INFO << "All attacks will be reported." << MsgStream::endl;
    } else {
	msgStr << MsgStream::INFO << "Only first attack will be reported." << MsgStream::endl;
    }

    // extracting pause_update_when_attack parameter
    if(config->nodeExists(CONFIGTAG_PauseUpdateWhenAttack) && !(config->getValue(CONFIGTAG_PauseUpdateWhenAttack).empty())) {
	pause_update_when_attack = atoi(config->getValue(CONFIGTAG_PauseUpdateWhenAttack).c_str());
	if((pause_update_when_attack < 0) || (pause_update_when_attack > 3)) {
	    msgStr << MsgStream::ERROR << "Invalid setting for " << CONFIGTAG_PauseUpdateWhenAttack << MsgStream::endl;
	    pause_update_when_attack = 0;
	}
    } else {
	pause_update_when_attack = 0;
    } 
    switch (pause_update_when_attack) {
	case 0:
	    msgStr.print(MsgStream::INFO, "Updates continue after a detected attack.");
	    break;
	case 1:
	    msgStr.print(MsgStream::INFO, "No updates if at least one test detected an attack.");
	    break;
	case 2:
	    msgStr.print(MsgStream::INFO, "No updates if all tests detected an attack.");
	    break;
	case 3:
	    msgStr.print(MsgStream::INFO, "CUSUM updates are stopped for every metric individually if at least one test detected an attack.");
	    break;
    }

    config->leaveNode();

    //
    // cusum section
    // 

    // extracting cusum parameters
    if (config->selectNodeIfExists(CONFIGTAG_CusumParams) && config->enterNodeIfNotEmpty()) {
	init_cusum_test(config);
	config->leaveNode();
    } else {
	enable_cusum_test = false;
    }

    //
    // wkp-test section
    // 

    if (config->selectNodeIfExists(CONFIGTAG_WkpParams) && config->enterNodeIfNotEmpty()) {
	init_wkp_test(config);
	config->leaveNode();

    } else {
	enable_wmw_test = false;
	enable_ks_test = false;
	enable_pcs_test = false;
    }

    // configuration overview
    std::string overview = config_overview();
    logStr << MsgStream::raw << MsgStream::FATAL << overview << MsgStream::endl;
    
    // create file which contains all relevant config params
    if (createFiles == true){
	chdir(output_dir.c_str());
	std::ofstream file("CONFIG");

	file << overview;

	file.close();
	chdir("..");
    }

    /* one should not forget to free "config" after use */
    delete config;
    msgStr.print(MsgStream::INFO, "Initialization complete.");

    // now everything is ready to begin monitoring:
    StatStore::beginMonitoring = true;

}


#ifdef IDMEF_SUPPORT_ENABLED
void Stat::update(XMLConfObj* xmlObj)
{
    std::cout << "Update received!" << std::endl;
    if (xmlObj->nodeExists("stop")) {
	std::cout << "-> stopping module..." << std::endl;
	stop();
    } else if (xmlObj->nodeExists("restart")) {
	std::cout << "-> restarting module..." << std::endl;
	restart();
    } else if (xmlObj->nodeExists("config")) {
	std::cout << "-> updating module configuration..." << std::endl;
    } else { // add your commands here
	std::cout << "-> unknown operation" << std::endl;
    }
}
#endif

// ================== FUNCTIONS USED BY init FUNCTION =================



void Stat::init_endpoints(XMLConfObj * config) 
{
    int endpointlist_maxsize;

    // extracting the KEY of the endpoints
    // and thus determining,
    // ONLINE MODE: which IPFIX_TYPEIDs we need to subscribe to
    // OFFLINE MODE: which endpoint members we need to set to 0
    if (config->nodeExists(CONFIGTAG_EndpointKey)) {
	msgStr << MsgStream::INFO << "Endpoint key is: ";
	config->enterNode(CONFIGTAG_EndpointKey);
	if(config->nodeExists(CONFIGTAG_EndpointKeyProtocol)) {
	    StatStore::protocolMonitoring = true;
#ifndef OFFLINE_ENABLED
	    subscribeTypeId(IPFIX_TYPEID_protocolIdentifier);
#endif
	    msgStr << "protocol ";
	}
	if(config->nodeExists(CONFIGTAG_EndpointKeyAddress)) {
	    StatStore::ipMonitoring = true;
#ifndef OFFLINE_ENABLED
	    subscribeTypeId(IPFIX_TYPEID_sourceIPv4Address);
	    subscribeTypeId(IPFIX_TYPEID_destinationIPv4Address);
#endif
	    msgStr << "address ";
	}
	if(config->nodeExists(CONFIGTAG_EndpointKeyPort)) {
	    StatStore::portMonitoring = true;
#ifndef OFFLINE_ENABLED
	    subscribeTypeId(IPFIX_TYPEID_sourceTransportPort);
	    subscribeTypeId(IPFIX_TYPEID_destinationTransportPort);
#endif
	    msgStr << "port ";
	}
	msgStr << MsgStream::endl;
	config->leaveNode();
    } else {
	msgStr << MsgStream::WARN << "No " << CONFIGTAG_EndpointKey << " parameter in XML config file! All traffic will be aggregated to endpoint 0.0.0.0:0|0" << MsgStream::endl;
    }

    // extract the MAXIMUM SIZE of the endpoint list
    // i. e. how many endpoints can be monitored
    if(config->nodeExists(CONFIGTAG_MaxEndpoints) && !(config->getValue(CONFIGTAG_MaxEndpoints).empty()))
	endpointlist_maxsize = atoi(config->getValue(CONFIGTAG_MaxEndpoints).c_str());
    else
	endpointlist_maxsize = DEFAULT_MaxEndpoints;
    msgStr << MsgStream::INFO << "Maximum number of endpoints is " << endpointlist_maxsize << MsgStream::endl;
    StatStore::endPointListMaxSize = endpointlist_maxsize;

    // OFFLINE MODE
#ifdef OFFLINE_ENABLED
    std::map<FilterEndPoint,int> endPointCount;
    uint16_t x_frequently_endpoints;
  
    // if parameter <x_frequently_endpoints> was provided,
    // read data from file and find the most X frequently appearing
    // EndPoints to create endPointFilter
    // otherwise use the endpoints_to_filter parameter
    if (config->nodeExists(CONFIGTAG_XFrequentEndpoints)) {
	if(!(config->getValue(CONFIGTAG_XFrequentEndpoints)).empty()) 
	    x_frequently_endpoints = atoi((config->getValue(CONFIGTAG_XFrequentEndpoints)).c_str());
	else
	    x_frequently_endpoints = DEFAULT_XFrequentEndpoints;
	msgStr << MsgStream::INFO << "Monitoring the " << x_frequently_endpoints << " most frequent endpoints." << MsgStream::endl;

	std::ifstream dataFile;
	dataFile.open(offlineFile.c_str());
	if (!dataFile) {
	    msgStr.print(MsgStream::FATAL, "Could't open offline file \"" + offlineFile + "\"!");
	    stop();
	    return;
	}

	std::string tmp;
	FilterEndPoint fep;
	while ( !dataFile.eof() && getline(dataFile, tmp) ) {

	    if (0 == strncmp("---",tmp.c_str(),3) )
		continue;

	    // extract endpoint-data
	    fep.fromString(tmp, false);

	    // AGGREGATION (specified by endpoint_key and netmask)
	    // as our data will be aggregated, the x_frequently_endpoints
	    // need to be aggregated, too
	    if (StatStore::ipMonitoring == false)
		fep.setIpAddress(IpAddress(0,0,0,0));
	    else
		// apply the global aggregation netmask to fep's ip address
		fep.applyNetmask(StatStore::netmask);
	    if (StatStore::portMonitoring == false)
		fep.setPortNr(0);
	    if (StatStore::protocolMonitoring == false)
		fep.setProtocolID(0);

	    // count number of appearings of each EndPoint
	    endPointCount[fep]++;
	}

	dataFile.close();

	// if all data was read
	// search the X most frequently appeared endpoints
	// but first check, if there are so many at all
	// and if there aren't, use only as many as exist!
	if (endPointCount.size() < x_frequently_endpoints) {
	    msgStr << MsgStream::INFO << "There are less then " << x_frequently_endpoints << " endpoints in the offline file. All endpoints will be monitored." << MsgStream::endl;
	    StatStore::monitorEveryEndPoint = true;
	    return;
	}

	msgStr << MsgStream::INFO << "The most frequent endpoints are: ";
	for (int j = 0; j < x_frequently_endpoints; j++) {
	    FilterEndPoint mostFrequentFEP;
	    int frequency = 0;
	    for (std::map<FilterEndPoint,int>::iterator i = endPointCount.begin(); i != endPointCount.end(); i++) {
		if (i->second > frequency) {
		    mostFrequentFEP = i->first;
		    frequency = i->second;
		}
	    }
	    StatStore::AddEndPointToFilter(mostFrequentFEP);
	    endPointCount.erase(mostFrequentFEP);
	    msgStr << mostFrequentFEP.toString() << " ";
	}
	msgStr << MsgStream::endl;

	return;
    }
#endif

    // ONLINE MODE + OFFLINE MODE (if no <x_frequently_endpoints> parameter is provided)
    // create endPointFilter from a file specified in XML config file
    // if no such file is specified --> MonitorEveryEndPoint = true
    // The endpoints in the file are in the format
    // ip1.ip2.ip3.ip4/netmask:port|protocol, e. g. 192.13.17.1/24:80|6
    if (config->nodeExists(CONFIGTAG_EndpointsToMonitor) && (config->getValue(CONFIGTAG_EndpointsToMonitor) != "all")) {
	std::ifstream f(config->getValue(CONFIGTAG_EndpointsToMonitor).c_str());
	if (f.is_open() == false) {
	    msgStr << MsgStream::ERROR << "Endpoints' file \"" << config->getValue(CONFIGTAG_EndpointsToMonitor) << "\" couldnt be opened! Exiting." << MsgStream::endl;
	    stop();
	    return;
	}
	std::string tmp;
	while ( !f.eof() && getline(f, tmp) ) {
	    FilterEndPoint fep;
	    fep.fromString(tmp, true); // with netmask applied
	    StatStore::AddEndPointToFilter(fep);
	}
    } else {
	logStr.print(MsgStream::INFO, "All endpoints will be monitored.");
	StatStore::monitorEveryEndPoint = true;
    }

}


void Stat::init_cusum_test(XMLConfObj * config) 
{
    // cusum_test
    enable_cusum_test = true;
    if (config->nodeExists(CONFIGTAG_CusumTest) && (config->getValue(CONFIGTAG_CusumTest) == "false")) {
	enable_cusum_test = false;
	msgStr.print(MsgStream::INFO, "CUSUM test is disabled.");
	return;
    }

    // amplitude_percentage
    if (config->nodeExists(CONFIGTAG_AmplitudePercentage) && !(config->getValue(CONFIGTAG_AmplitudePercentage)).empty()) {
	amplitude_percentage = atof(config->getValue(CONFIGTAG_AmplitudePercentage).c_str());
	if(!(amplitude_percentage > 0)) {
	    msgStr.print(MsgStream::FATAL, "CUSUM amplitude percentage out of valid range (>0)");
	    stop();
	    return;
	}
    } else {
	amplitude_percentage = DEFAULT_AmplitudePercentage;
    }
    msgStr << MsgStream::INFO << "CUSUM amplitude percentage is " << amplitude_percentage << MsgStream::endl;

    // cusum_learning_phase
    if (config->nodeExists(CONFIGTAG_CusumLearningPhase) && !(config->getValue(CONFIGTAG_CusumLearningPhase)).empty()) {
	cusum_learning_phase = atoi(config->getValue(CONFIGTAG_CusumLearningPhase).c_str());
	if(!(cusum_learning_phase > 0))
	    cusum_learning_phase = DEFAULT_CusumLearningPhase;
    } else {
	cusum_learning_phase = DEFAULT_CusumLearningPhase;
    }
    msgStr << MsgStream::INFO << "CUSUM learning phase is " << cusum_learning_phase << MsgStream::endl;

    // repetition factor
    if (config->nodeExists(CONFIGTAG_RepetitionFactor) && !(config->getValue(CONFIGTAG_RepetitionFactor)).empty()) {
	repetition_factor = atoi(config->getValue(CONFIGTAG_RepetitionFactor).c_str());
	if(!(repetition_factor > 0))
	    repetition_factor = DEFAULT_RepetitionFactor;
    } else {
	repetition_factor = DEFAULT_RepetitionFactor;
    }
    msgStr << MsgStream::INFO << "CUSUM repetition factor is " << repetition_factor << MsgStream::endl;

    // smoothing constant for updating mean and variance per EWMA
    if (config->nodeExists(CONFIGTAG_SmoothingConstant) && !(config->getValue(CONFIGTAG_SmoothingConstant)).empty()) {
	smoothing_constant = atof(config->getValue(CONFIGTAG_SmoothingConstant).c_str());
	if((smoothing_constant < 0) || (smoothing_constant > 1)) {
	    msgStr.print(MsgStream::FATAL, "CUSUM smoothing constant out of valid range [0;1]");
	    stop();
	    return;
	}
    } else {
	smoothing_constant = DEFAULT_SmoothingConstant;
    }
    msgStr << MsgStream::INFO << "CUSUM smoothing constant is " << smoothing_constant << MsgStream::endl;

    // log_variance_ewma
    log_variance_ewma = false;
    if (config->nodeExists(CONFIGTAG_LogVarianceEWMA) && (config->getValue(CONFIGTAG_LogVarianceEWMA) != "false")) {
	log_variance_ewma = true;
	msgStr.print(MsgStream::INFO, "CUSUM deploys EWMA on log(variance).");
    }

}

void Stat::init_wkp_test(XMLConfObj * config) {
    // extracting type of test
    // (Wilcoxon and/or Kolmogorov and/or Pearson chi-square)
    enable_wmw_test = true;
    if (config->nodeExists(CONFIGTAG_WMWTest) && (config->getValue(CONFIGTAG_WMWTest) == "false")) {
	enable_wmw_test = false;
	msgStr.print(MsgStream::WARN, "Wilcoxon-Mann-Whitney test is disabled.");
    }

    enable_ks_test = true;
    if (config->nodeExists(CONFIGTAG_KSTest) && (config->getValue(CONFIGTAG_KSTest) == "false")) {
	enable_ks_test = false;
	msgStr.print(MsgStream::WARN, "Kolmogorov-Smirnov test is disabled.");
    }

    enable_pcs_test = true;
    if (config->nodeExists(CONFIGTAG_PCSTest) && (config->getValue(CONFIGTAG_PCSTest) == "false")) {
	enable_pcs_test = false;
	msgStr.print(MsgStream::WARN, "Pearson's chi-square test is disabled.");
    }

    // no need for further initialization of wkp-params if
    // none of the three tests is enabled
    if(!(enable_wkp_test = (enable_wmw_test || enable_ks_test || enable_pcs_test)))
	return;

    // extracting size of sample_old
    if (config->nodeExists(CONFIGTAG_OldSampleSize) && !(config->getValue(CONFIGTAG_OldSampleSize)).empty()) {
	sample_old_size = atoi( config->getValue(CONFIGTAG_OldSampleSize).c_str() );
	if(!(sample_old_size > 0))
	    sample_old_size = DEFAULT_OldSampleSize;
    } else {
	sample_old_size = DEFAULT_OldSampleSize;
    }
    msgStr << MsgStream::INFO << "WKP old sample size is " << sample_old_size << MsgStream::endl;

    // extracting size of sample_new
    if (config->nodeExists(CONFIGTAG_NewSampleSize) && !(config->getValue(CONFIGTAG_NewSampleSize)).empty()) {
	sample_new_size = atoi( config->getValue(CONFIGTAG_NewSampleSize).c_str() );
	if(!(sample_new_size > 0))
	    sample_new_size = DEFAULT_NewSampleSize;
    } else {
	sample_new_size = DEFAULT_NewSampleSize;
    }
    msgStr << MsgStream::INFO << "WKP new sample size is " << sample_new_size << MsgStream::endl;

    // extracting one/two-sided parameter for the wmw test
    // pcs test and ks test are always two-sided (in the sense that they detect 
    // increases and decreases in the distribution).
    wmw_two_sided = true;
    if (config->nodeExists(CONFIGTAG_WMWOneSided) && (config->getValue(CONFIGTAG_WMWOneSided) != "false")) {
	wmw_two_sided = false;
	msgStr.print(MsgStream::INFO, "WMW test is used as one-sided test.");
    }

    // extracting significance level parameter
    if (config->nodeExists(CONFIGTAG_SignificanceLevel) && !(config->getValue(CONFIGTAG_SignificanceLevel)).empty()) {
	significance_level = atof(config->getValue(CONFIGTAG_SignificanceLevel).c_str());
	if((significance_level < 0) || (significance_level > 1)) {
	    msgStr.print(MsgStream::FATAL, "WKP significance level out of valid range [0;1]");
	    stop();
	    return;
	}
    } else {
	significance_level = DEFAULT_SignificanceLevel;
    }
    msgStr << MsgStream::INFO << "WKP significance level is " << significance_level << MsgStream::endl;

}

std::string Stat::config_overview()
{
    std::stringstream config;

    if (use_pca == true) {
	config << "PCA enabled\n"
	    << "pca_learning_phase = " << pca_learning_phase << "\n"
	    << "use_correlation_matrix = " << ((use_correlation_matrix == true)?"true":"false") << "\n\n";
    }
    else
	config << "PCA disabled\n\n";
    
    config << "METRICS:\n";
    for(std::vector<MetricData>::iterator val = metrics.begin(); val != metrics.end(); val++)
	config << val->name << "\n";
    config << "endpoint_key = " << (StatStore::ipMonitoring?"address ":"") << (StatStore::portMonitoring?"port ":"") << (StatStore::protocolMonitoring?"protocol":"") << "\n\n";

    config << "ENDPOINTS:\n";
    config << StatStore::EndPointFilters();
    config << "netmask = " << StatStore::netmask << "\n\n";
    
    config << "ALERTING:\n";
    config << "noise_thresholds: packets = " << noise_threshold_packets
	<< "; bytes = " << noise_threshold_bytes << "\n";
    config << "pause_update_when_attack = " << pause_update_when_attack << "\n"
	<< "report_only_first_attack = " << report_only_first_attack << "\n"
	<< "stat_test_frequency = " << stat_test_frequency << "\n\n";
    
    config << "CUSUM:\n";
    if (enable_cusum_test == true) {
	config << "amplitude_percentage = " << amplitude_percentage << "\n"
	    << "repetition_factor = " << repetition_factor << "\n"
	    << "cusum_learning_phase = " << cusum_learning_phase << "\n"
	    << "smoothing_constant = " << smoothing_constant << "\n"
	    << "log_variance_ewma = " << (log_variance_ewma?"true":"false") << "\n\n";
    } else {
	config << "disabled\n\n";
    }
    
    config << "WKP:\n";
    if (enable_wkp_test == true) {
	config << "wmw test: " << (enable_wmw_test?"enabled":"disabled") << "\n";
	config << "ks test: " << (enable_ks_test?"enabled":"disabled") << "\n";
	config << "pcs test: " << (enable_pcs_test?"enabled":"disabled") << "\n";
	config << "sample_old_size = " << sample_old_size << "\n"
	    << "sample_new_size = " << sample_new_size << "\n"
	    << "wmw_two_sided = " << wmw_two_sided << "\n"
	    << "significance_level = " << significance_level << "\n";
    } else {
	config << "disabled\n\n";
    }

    return config.str();
}

// ============================= TEST FUNCTION ===============================
void Stat::test(StatStore * store) {

#ifdef IDMEF_SUPPORT_ENABLED
    idmefMessage = getNewIdmefMessage("wkp-module", "statistical anomaly detection");
#endif

    std::map<EndPoint,Info> Data = store->getData();

#ifndef OFFLINE_ENABLED
    if (storefile.is_open() == true)
	// store data storage for offline use
	storefile << Data << "--- " << test_counter << std::endl << std::flush;
#endif

    // Dumping empty records:
    if (Data.empty()==true) {
	logStr.rawPrint(MsgStream::WARN, "INFORMATION: Got empty record; dumping it and waiting for another record");
	test_counter++;
	return;
    }

    logStr << MsgStream::raw << MsgStream::FATAL << "####################################################\n"
	<< "########## Stat::test(...)-call number: " << test_counter << " ##########\n"
	<< "####################################################" << MsgStream::endl;

    std::map<EndPoint,Info>::iterator Data_it = Data.begin();

    std::map<EndPoint,Info> PreviousData = store->getPreviousData();
    //std::map<EndPoint,Info> PreviousData = store->getPreviousDataFromFile();
    // Needed for extraction of packets(t)-packets(t-1) and bytes(t)-bytes(t-1)
    // Holds information about the Info used in the last call to test()
    Info prev;

    // 1) LEARN/UPDATE PHASE
    // Parsing data to see whether the recorded EndPoints already exist
    // in our  "std::map<EndPoint, Params> EndpointParams" container.
    // If not, then we add it as a new pair <EndPoint, Params>.
    // If yes, then we update the corresponding entry using
    // std::vector<int64_t> extracted data.
    logStr.rawPrint (MsgStream::WARN, "#### LEARN/UPDATE PHASE");

    // for every EndPoint, extract the data
    while (Data_it != Data.end()) {
	logStr << MsgStream::raw << MsgStream::WARN << "[[ " << Data_it->first.toString() << " ]]" << MsgStream::endl;

	prev = PreviousData[Data_it->first];
	// it doesn't matter much if Data_it->first is an EndPoint that exists
	// only in Data, but not in PreviousData, because
	// PreviousData[Data_it->first] will automaticaly be an Info structure
	// with all fields set to 0.

	std::vector<int64_t> metric_data;
	std::vector<int64_t> pca_metric_data;

	std::map<EndPoint, Params>::iterator EndpointParams_it =
	    EndpointParams.find(Data_it->first);

	if (EndpointParams_it == EndpointParams.end()) {
	    // We didn't find the recorded EndPoint Data_it->first
	    // in our container "EndpointParams"; that means it's a new one,
	    // so we just add it in "EndpointParams"; there will not be jeopardy
	    // of memory exhaustion through endless growth of the "EndpointParams" map
	    // as limits are implemented in the StatStore class (EndPointListMaxSize)

	    // Initialization
	    Params P;
	    P.correspondingEndPoint = (Data_it->first).toString();
	    if (enable_wkp_test == true) {
		for (int i=0; i < metrics.size(); i++) {
		    (P.last_wmw_test_was_attack).push_back(false);
		    (P.last_ks_test_was_attack).push_back(false);
		    (P.last_pcs_test_was_attack).push_back(false);
		    (P.wmw_alarms).push_back(0);
		    (P.ks_alarms).push_back(0);
		    (P.pcs_alarms).push_back(0);
		}
	    }
	    if (enable_cusum_test == true) {
		for (int i = 0; i != metrics.size(); i++) {
		    (P.mean).push_back(0);
		    (P.variance).push_back(0.0);
		    (P.N).push_back(0);
		    (P.beta).push_back(0.0);
		    (P.g).push_back(0.0);
		    (P.last_cusum_test_was_attack).push_back(false);
		    (P.X_curr).push_back(0);
		    (P.cusum_alarms).push_back(0);
		}
	    }

	    logStr.rawPrint(MsgStream::WARN , "Add as new monitored endpoint");
	    if (use_pca == true) {
		// initialize PCA parameters
		P.init(metrics.size());
		pca_metric_data = extract_pca_data(P, Data_it->second, prev);
	    }
	    else {
		metric_data = extract_data(Data_it->second, prev);
		if (enable_wkp_test == true) {
		    (P.Old).push_back(metric_data);
		    if(logStr.getLogLevel() >= MsgStream::INFO) {
			std::stringstream tmp;
			tmp << "  (WKP): with first element of sample_old: " << metric_data;
			logStr.rawPrint(MsgStream::INFO, tmp.str());
		    }
		}
		if (enable_cusum_test == true) {
		    // add first value of each metric to the sum, which is needed
		    // only to calculate the initial mean and variance after cusum_learning_phase
		    // is over
		    for (int i = 0; i != metric_data.size(); i++) {
			P.mean.at(i) = metric_data.at(i);
			P.variance.at(i) = metric_data.at(i) * metric_data.at(i);
		    }
		    P.cusum_learning_phase_nr = 1;
		    logStr.rawPrint(MsgStream::WARN, "  (CUSUM): with initial values for mean and variance of metrics.");
		}
	    }

	    EndpointParams[Data_it->first] = P;
	}
	else {
	    // We found the recorded EndPoint Data_it->first
	    // in our container "EndpointParams"; so we update the data
	    if (use_pca == true) {
		pca_metric_data = extract_pca_data(EndpointParams_it->second, Data_it->second, prev);
		// Updates for pca metrics have to wait for the pca learning phase
		// needed to calculate the eigenvectors
		// NOTE: The overall learning phases for the tests will thus sum up to
		// WKP: learning_phase_pca + learning_phase_samples
		// Cusum: learning_phase_pca + cusum_learning_phase
		if ((EndpointParams_it->second).pca_ready == false)
		    logStr.rawPrint(MsgStream::WARN, "Learning phase for PCA ...");
		// this case happens exactly one time: when PCA is ready for the first time
		else if ((EndpointParams_it->second).pca_ready == true && pca_metric_data.empty() == true) {
		    logStr.rawPrint(MsgStream::WARN, "PCA learning phase is over! PCA is now ready!");
		    if (logStr.getLogLevel() == MsgStream::DEBUG) {
			logStr << MsgStream::raw << MsgStream::DEBUG 
			    << "Calculated " << ((use_correlation_matrix == true)?"correlation matrix:":"covariance matrix:\n");
			for (int i = 0; i < metrics.size(); i++) {
			    for (int j = 0; j < metrics.size(); j++)
				logStr << gsl_matrix_get((EndpointParams_it->second).cov,i,j) << "\t";
			    logStr << "\n";
			}
			logStr << MsgStream::endl;
		    }
		}
		else {
		    if (enable_wkp_test == true)
			wkp_update ( EndpointParams_it->second, pca_metric_data );
		    if (enable_cusum_test == true)
			cusum_update ( EndpointParams_it->second, pca_metric_data );
		}
	    }
	    else {
		metric_data = extract_data(Data_it->second, prev);
		if (enable_wkp_test == true)
		    wkp_update ( EndpointParams_it->second, metric_data );
		if (enable_cusum_test == true)
		    cusum_update ( EndpointParams_it->second, metric_data );
	    }
	}

	// Create metric files, if wished so
	// (this can be done here, because metrics are the same for both tests)
	// in case of pca: don't create file until learning phase is over
	if ((createFiles == true) && ((use_pca == false) || (pca_metric_data.size() > 0))) {

	    std::string fname;

	    fname = (Data_it->first).toString() + "_metrics.txt";

	    chdir(output_dir.c_str());

	    std::ofstream file(fname.c_str(),std::ios_base::app);

	    // are we at the beginning of the file?
	    // if yes, write the metric names to the file ...
	    long pos;
	    pos = file.tellp();

	    if (use_pca == true) {
		if (pos == 0) {
		    file << "# ";
		    for (int i = 0; i < pca_metric_data.size(); i++)
			file << "pca_comp_" << i << "\t";
		    file << "Test-Run" << "\n";
		}
		for (int i = 0; i < pca_metric_data.size(); i++)
		    file << pca_metric_data.at(i) << "\t";
		file << test_counter << "\n";
	    }
	    else {
		if (pos == 0) {
		    file << "# ";
		    for(std::vector<MetricData>::iterator val = metrics.begin(); val != metrics.end(); val++)
			file << val->name << "\t";
		    file << "Test-Run" << "\n";
		}
		for (int i = 0; i < metric_data.size(); i++)
		    file << metric_data.at(i) << "\t";
		file << test_counter << "\n";
	    }

	    file.close();

	    chdir("..");
	}

	Data_it++;
    }

    // 1.5) MAP PRINTING (OPTIONAL, DEPENDS ON VERBOSITY SETTINGS)

    // how many endpoints do we already monitor?
    int ep_nr;
    ep_nr = EndpointParams.size();

    if((unsigned)logStr.getLogLevel() >= (unsigned)MsgStream::INFO) {
	logStr << MsgStream::raw << MsgStream::INFO << "#### STATE OF ALL MONITORED ENDPOINTS (" << ep_nr << "):" << MsgStream::endl;

	std::map<EndPoint,Params>::iterator EndpointParams_it = EndpointParams.begin();
	while (EndpointParams_it != EndpointParams.end()) {
	    logStr.rawPrint(MsgStream::INFO, "[[ " + EndpointParams_it->first.toString() + " ]]");
	    if (enable_wkp_test == true) {
		std::stringstream tmp;
		tmp << "   sample_old (" << (EndpointParams_it->second).Old.size() << ") : " << (EndpointParams_it->second).Old << "\n"
												 << "   sample_new (" << (EndpointParams_it->second).New.size() << ") : " << (EndpointParams_it->second).New;
		logStr << MsgStream::raw << MsgStream::INFO << " (WKP):\n" << tmp.str() << MsgStream::endl;
	    }
	    if (enable_cusum_test == true) {
		std::stringstream tmp;
		if(logStr.getLogLevel() >= MsgStream::INFO) {
		    logStr << MsgStream::raw << MsgStream::INFO << " (CUSUM):\n" << "   mean: ";
		    for (int i = 0; i != (EndpointParams_it->second).mean.size(); i++)
			logStr << (EndpointParams_it->second).mean.at(i) << " ";
		    logStr << MsgStream::endl;
		    logStr << MsgStream::raw << MsgStream::INFO << "   standard deviation: ";
		    for (int i = 0; i != (EndpointParams_it->second).mean.size(); i++)
			logStr << sqrt((EndpointParams_it->second).variance.at(i)) << " ";
		    logStr << MsgStream::endl;
		    logStr << MsgStream::raw << MsgStream::INFO << "   g: ";
		    for (int i = 0; i != (EndpointParams_it->second).mean.size(); i++)
			logStr << (EndpointParams_it->second).g.at(i) << " ";
		    logStr << MsgStream::endl;
		}
	    }
	    EndpointParams_it++;
	}
    }



    // 2) STATISTICAL TEST
    // (OPTIONAL, DEPENDS ON HOW OFTEN THE USER WISHES TO DO IT)
    bool MakeStatTest;
    if (stat_test_frequency == 0)
	MakeStatTest = false;
    else
	MakeStatTest = (test_counter % stat_test_frequency == 0);

    // (i.e., if stat_test_frequency=X, then the test will be conducted when
    // test_counter is a multiple of X; if X=0, then the test will never
    // be conducted)

    if (MakeStatTest == true) {
	// We begin testing as soon as possible, i.e.
	// for WKP: as soon as a sample is big enough to test, i.e. when its
	// learning phase is over.
	// for CUSUM: as soon as we have enough values for calculating the initial
	// values of mean and variance
	// The other endpoints in the EndpointParams map are let learning.

	std::map<EndPoint,Params>::iterator EndpointParams_it = EndpointParams.begin();
	while (EndpointParams_it != EndpointParams.end()) {
	    if ( enable_wkp_test == true && ((EndpointParams_it->second).New).size() == sample_new_size ) {
		// i.e. learning phase over
		logStr.rawPrint(MsgStream::FATAL, "\n#### WKP TESTS for EndPoint [[ " + EndpointParams_it->first.toString() + " ]]");
		wkp_test ( EndpointParams_it->second );
	    }
	    if ( enable_cusum_test == true && (EndpointParams_it->second).ready_to_test == true ) {
		// i.e. learning phase for cusum is over and it has an initial value
		logStr.rawPrint(MsgStream::FATAL, "\n#### CUSUM TESTS for EndPoint [[ " + EndpointParams_it->first.toString() + " ]]");
		cusum_test ( EndpointParams_it->second );
	    }
	    EndpointParams_it++;
	}
    }

    test_counter++;
    // don't forget to free the store-object!
    delete store;
    return;

}



// =================== FUNCTIONS USED BY THE TEST FUNCTION ====================

// calculates the desired metric and returns true on success
int64_t Stat::get_metric (const Info & info, Metric m)
{
    switch (m) {
	case PACKETS_IN:
	    if (info.packets_in >= noise_threshold_packets)
		return info.packets_in;
	    else
		return 0;
	    break;

	case PACKETS_OUT:
	    if (info.packets_out >= noise_threshold_packets)
		return info.packets_out;
	    else
		return 0;
	    break;

	case BYTES_IN:
	    if (info.bytes_in >= noise_threshold_bytes)
		return info.bytes_in;
	    else
		return 0;
	    break;

	case BYTES_OUT:
	    if (info.bytes_out >= noise_threshold_bytes)
		return info.bytes_out;
	    else
		return 0;
	    break;

	case RECORDS_IN:
	    return info.records_in;
	    break;

	case RECORDS_OUT:
	    return info.records_out;
	    break;

	case BYTES_IN_PER_PACKET_IN:
	    if ( info.packets_in >= noise_threshold_packets
		    || info.bytes_in   >= noise_threshold_bytes ) {
		if (info.packets_in == 0)
		    return 0;
		else
		    return info.bytes_in / info.packets_in;
	    }
	    else
		return 0;
	    break;

	case BYTES_OUT_PER_PACKET_OUT:
	    if (info.packets_out >= noise_threshold_packets ||
		    info.bytes_out   >= noise_threshold_bytes ) {
		if (info.packets_out == 0)
		    return 0;
		else
		    return info.bytes_out / info.packets_out;
	    }
	    else
		return 0;
	    break;

	case PACKETS_IN_PER_RECORD_IN:
	    if ( info.packets_in >= noise_threshold_packets) {
		if (info.records_in == 0)
		    return 0;
		else
		    return info.packets_in / info.records_in;
	    }
	    else
		return 0;
	    break;

	case PACKETS_OUT_PER_RECORD_OUT:
	    if ( info.packets_out >= noise_threshold_packets) {
		if (info.records_out == 0)
		    return 0;
		else
		    return info.packets_out / info.records_out;
	    }
	    else
		return 0;
	    break;

	case BYTES_IN_PER_RECORD_IN:
	    if ( info.bytes_in >= noise_threshold_bytes) {
		if (info.records_in == 0)
		    return 0;
		else
		    return info.bytes_in / info.records_in;
	    }
	    else
		return 0;
	    break;

	case BYTES_OUT_PER_RECORD_OUT:
	    if ( info.bytes_out >= noise_threshold_bytes) {
		if (info.records_out == 0)
		    return 0;
		else
		    return info.bytes_out / info.records_out;
	    }
	    else
		return 0;
	    break;

	case PACKETS_OUT_MINUS_PACKETS_IN:
	    if (info.packets_out >= noise_threshold_packets
		    || info.packets_in  >= noise_threshold_packets )
		return info.packets_out - info.packets_in;
	    else
		return 0;
	    break;

	case BYTES_OUT_MINUS_BYTES_IN:
	    if (info.bytes_out >= noise_threshold_bytes
		    || info.bytes_in  >= noise_threshold_bytes )
		return info.bytes_out - info.bytes_in;
	    else
		return 0;
	    break;

	case RECORDS_OUT_MINUS_RECORDS_IN:
	    return info.records_out - info.records_in;
	    break;

	case PACKETS_IN_MINUS_PACKETS_OUT:
	    if (info.packets_out >= noise_threshold_packets
		    || info.packets_in  >= noise_threshold_packets )
		return info.packets_in - info.packets_out;
	    else
		return 0;
	    break;

	case BYTES_IN_MINUS_BYTES_OUT:
	    if (info.bytes_out >= noise_threshold_bytes
		    || info.bytes_in  >= noise_threshold_bytes )
		return info.bytes_in - info.bytes_out;
	    else
		return 0;
	    break;

	case RECORDS_IN_MINUS_RECORDS_OUT:
	    return info.records_in - info.records_out;
	    break;

	default:
	    msgStr.print(MsgStream::FATAL, "Invalid metric, this should never happen.");
	    stop();
    }

}

// extracts interesting data from StatStore according to metrics:
std::vector<int64_t>  Stat::extract_data (const Info & info, const Info & prev) {

    std::vector<int64_t>  result;
    int64_t new_value;

    std::vector<MetricData>::iterator it = metrics.begin();

    while (it != metrics.end() ) {

	new_value = get_metric(info, it->type);

	if(it->applyQuasiDifference)
	    new_value = (int64_t) (new_value - it->factor * get_metric(prev, it->type));
	if(it->absolute)
	    new_value = abs(new_value);
	if(it->negate)
	    new_value = -new_value;

	result.push_back(new_value);

	it++;
    }

    return result;
}

// needed for pca to extract the new values (for learning and testing)
std::vector<int64_t> Stat::extract_pca_data (Params & P, const Info & info, const Info & prev) {

    std::vector<int64_t> result;

    // learning phase
    if (P.pca_ready == false) {
	if (P.pca_learning_phase_nr < pca_learning_phase) {
	    // update sumsOfMetrics and sumsOfProducts
	    std::vector<int64_t> v = extract_data(info, prev);
	    for (int i = 0; i < metrics.size(); i++) {
		P.sumsOfMetrics.at(i) += v.at(i);
		for (int j = 0; j <= i; j++) {
		    // sumsOfProducts is a matrix which holds all the sums
		    // of the product of each two metrics
		    P.sumsOfProducts.at(i).at(j) += v.at(i)*v.at(j);
		    P.sumsOfProducts.at(j).at(i) = P.sumsOfProducts.at(i).at(j);
		}
	    }

	    P.pca_learning_phase_nr++;
	    return result; // empty
	}
	// end of learning phase
	else if (P.pca_learning_phase_nr == pca_learning_phase) {

	    // calculate covariance matrix
	    for (int i = 0; i < metrics.size(); i++) {
		for (int j = 0; j < metrics.size(); j++) {
		    gsl_matrix_set(P.cov,i,j,covariance(P.sumsOfProducts.at(i).at(j),P.sumsOfMetrics.at(i),P.sumsOfMetrics.at(j)));
		}
	    }

	    // if wanted, calculate correlation matrix based on the covariance matrix
	    if (use_correlation_matrix == true) {
		for (int i = 0; i < metrics.size(); i++) {
		    // calculate standard deviation of metric i
		    double sd_i;
		    sd_i = standard_deviation(P.sumsOfProducts.at(i).at(i),P.sumsOfMetrics.at(i));
		    for (int j = 0; j < metrics.size(); j++) {
			// calculate standard deviation of metric j
			double sd_j;
			if (i != j) {
			    sd_j = standard_deviation(P.sumsOfProducts.at(j).at(j),P.sumsOfMetrics.at(j));
			    gsl_matrix_set(P.cov,i,j,gsl_matrix_get(P.cov,i,j)/(sd_i*sd_j));
			    // store the standard deviations (we need them later to normalize new incoming data)
			    P.stddevs.at(i) = sd_i;
			    P.stddevs.at(j) = sd_j;
			}
			// elements on the main diagonal are equal to 1
			else
			    gsl_matrix_set(P.cov,i,j,1.0);
		    }
		}
	    }

	    //       for (int i = 0; i < metrics.size(); i++) {
	    //         std::cout << "stddev(" << i << ") : " << P.stddevs.at(i) << std::endl;
	    //       }
	    //       std::cout << std::endl;


	    // calculate eigenvectors and -values
	    gsl_vector *eval = gsl_vector_alloc (metrics.size());
	    // some workspace needed for computation
	    gsl_eigen_symmv_workspace * w = gsl_eigen_symmv_alloc (metrics.size());
	    // computation of eigenvectors (evec) and -values (eval) from
	    // covariance or correlation matrix (cov)
	    gsl_eigen_symmv (P.cov, eval, P.evec, w);
	    gsl_eigen_symmv_free (w);
	    // sort the eigenvectors by their corresponding eigenvalue
	    gsl_eigen_symmv_sort (eval, P.evec, GSL_EIGEN_SORT_VAL_DESC);
	    gsl_vector_free (eval);

	    // now, we have our components stored in each column of
	    // evec, first column = most important, last column = least important

	    // From now on, matrix evec can be used to transform the new arriving data

	    P.pca_ready = true; // so this code will never be visited again

	    return result; // empty
	}
    }

    // testing phase

    // fetch new metric data
    std::vector<int64_t> v = extract_data(info,prev);
    // transform it into a matrix (needed for multiplication)
    // X*1 matrix with X = #metrics,
    gsl_matrix *new_metric_data = gsl_matrix_calloc (metrics.size(),1);
    if (use_correlation_matrix == true) {
	// Normalization: Divide the metric values by the stddevs, if correlation matrix is used
	for (int i = 0; i < metrics.size(); i++)
	    gsl_matrix_set(new_metric_data,i,0,((double)(v.at(i)) / P.stddevs.at(i)));
    } else  {
	for (int i = 0; i < metrics.size(); i++)
	    gsl_matrix_set(new_metric_data,i,0,v.at(i));
    }

    // matrix multiplication to get the transformed data
    // transformed_data = evec^T * data
    gsl_matrix *transformed_metric_data = gsl_matrix_calloc (metrics.size(),1);
    gsl_blas_dgemm (CblasTrans, CblasNoTrans,
	    1.0, P.evec, new_metric_data,
	    0.0, transformed_metric_data);

    // transform the matrix with the transformed data back into a vector
    if (use_correlation_matrix == true) {
	for (int i = 0; i < metrics.size(); i++)
	    result.push_back((int64_t) (gsl_matrix_get(transformed_metric_data,i,0) * 100)); // *100 to reduce rounding error
    }
    else {
	for (int i = 0; i < metrics.size(); i++)
	    result.push_back((int64_t) gsl_matrix_get(transformed_metric_data,i,0));
    }

    gsl_matrix_free(new_metric_data);
    gsl_matrix_free(transformed_metric_data);

    return result; // filled with new transformed values
}


// -------------------------- LEARN/UPDATE FUNCTION ---------------------------

// learn/update function for samples (called everytime test() is called)
//
void Stat::wkp_update ( Params & P, const std::vector<int64_t> & new_value ) {

    // Learning phase?
    if (P.Old.size() != sample_old_size) {

	P.Old.push_back(new_value);

	logStr.rawPrint(MsgStream::WARN, "  (WKP): Learning phase for sample_old ...");
	if((unsigned)logStr.getLogLevel() >= (unsigned)MsgStream::INFO) {
	    std::stringstream tmp;
	    tmp << "   sample_old: " << P.Old << "\n   sample_new: " << P.New;
	    logStr.rawPrint(MsgStream::INFO, tmp.str());
	}

	return;
    }
    else if (P.New.size() != sample_new_size) {

	P.New.push_back(new_value);

	logStr.rawPrint(MsgStream::WARN, "  (WKP): Learning phase for sample_new...");
	if((unsigned)logStr.getLogLevel() >= (unsigned)MsgStream::INFO) {
	    std::stringstream tmp;
	    tmp << "   sample_old: " << P.Old << "\n   sample_new: " << P.New;
	    logStr.rawPrint(MsgStream::INFO, tmp.str());
	}

	return;
    }

    // Learning phase over: update

    // pausing update for old sample

    bool at_least_one_wmw_test_was_attack = false;
    bool at_least_one_ks_test_was_attack = false;
    bool at_least_one_pcs_test_was_attack = false;
    for (int i = 0; i < P.last_wmw_test_was_attack.size(); i++) {
	if (P.last_wmw_test_was_attack.at(i) == true)
	    at_least_one_wmw_test_was_attack = true;
	if (P.last_ks_test_was_attack.at(i) == true)
	    at_least_one_ks_test_was_attack = true;
	if (P.last_pcs_test_was_attack.at(i) == true)
	    at_least_one_pcs_test_was_attack = true;
    }
    /*
       bool all_wmw_tests_were_attacks = true;
       bool all_ks_tests_were_attacks = true;
       bool all_pcs_tests_were_attacks = true;
       for (int i = 0; i < P.last_wmw_test_was_attack.size(); i++) {
       if (P.last_wmw_test_was_attack.at(i) == false)
       all_wmw_tests_were_attacks = false;
       if (P.last_ks_test_was_attack.at(i) == false)
       all_ks_tests_were_attacks = false;
       if (P.last_pcs_test_was_attack.at(i) == false)
       all_pcs_tests_were_attacks = false;
       }
       */

    if ( (pause_update_when_attack == 1
		&& ( at_least_one_wmw_test_was_attack == true
		    || at_least_one_ks_test_was_attack  == true
		    || at_least_one_pcs_test_was_attack == true ))
	    || (pause_update_when_attack == 2
		&& ( at_least_one_wmw_test_was_attack == true
		    && at_least_one_ks_test_was_attack  == true
		    && at_least_one_pcs_test_was_attack == true )) ) {

	P.New.pop_front();
	P.New.push_back(new_value);

	logStr.rawPrint(MsgStream::WARN, "  (WKP): Update done (for new sample only)");
	if((unsigned)logStr.getLogLevel() >= (unsigned)MsgStream::INFO) {
	    std::stringstream tmp;
	    tmp << "   sample_old: " << P.Old << "\n   sample_new: " << P.New;
	    logStr.rawPrint(MsgStream::INFO, tmp.str());
	}
    }
    // if parameter is 0 (or 3) or there was no attack detected
    // update both samples
    else {
	P.Old.pop_front();
	P.Old.push_back(P.New.front());
	P.New.pop_front();
	P.New.push_back(new_value);

	logStr.rawPrint(MsgStream::WARN, "  (WKP): Update done (for both samples)");
	if((unsigned)logStr.getLogLevel() >= (unsigned)MsgStream::INFO) {
	    std::stringstream tmp;
	    tmp << "   sample_old: " << P.Old << "\n   sample_new: " << P.New;
	    logStr.rawPrint(MsgStream::INFO, tmp.str());
	}
    }

    P.wkp_updated = true;

    return;
}


// and the update function for the cusum-test
void Stat::cusum_update ( Params & P, const std::vector<int64_t> & new_value ) {

    // getting here means that we received data for this endpoint and apply the necessary updates for cusum
    P.cusum_updated = true;

    // Learning phase for cusum?
    // that means, we dont have enough values to calculate mean and variance
    // until now (and so cannot perform the cusum test)
    if (P.ready_to_test == false) {
	if (P.cusum_learning_phase_nr < cusum_learning_phase) {

	    // update the sum and sum of squares of the values of each metric
	    for (int i = 0; i != P.mean.size(); i++) {
		P.mean.at(i) += new_value.at(i); // division by cusum_learning_phase is done later
		P.variance.at(i) += new_value.at(i)*new_value.at(i);
	    }

	    logStr.rawPrint(MsgStream::WARN, "  (CUSUM): Learning phase for mean and variance ...");

	    P.cusum_learning_phase_nr++;

	    return;
	}
	// ok, we have enough values (P.cusum_learning_phase_nr == cusum_learning_phase)
	// Calculate initial mean and variance
	// and set ready_to_test-flag to true (so we never visit this
	// code here again for the current endpoint)

	logStr.rawPrint(MsgStream::WARN, "  (CUSUM): Learning phase for mean and variance is over.");
	logStr.rawPrint(MsgStream::INFO, "   Calculated initial mean and standard deviation values:");

	for (int i = 0; i != P.mean.size(); i++) {
	    // calculate initial mean and variance from sum and sum of squares
	    P.mean.at(i) = P.mean.at(i) / cusum_learning_phase;
	    P.variance.at(i) = (P.variance.at(i) - (P.mean.at(i)*P.mean.at(i))/cusum_learning_phase)/(cusum_learning_phase-1);
	    P.X_curr.at(i) = new_value.at(i);
	    logStr << MsgStream::raw << MsgStream::INFO << P.mean.at(i) << "," << sqrt(P.variance.at(i)) << MsgStream::endl;
	}

	P.ready_to_test = true;
	return;
    }

    // pausing update for mean and variance (depending on pause_update parameter)
    bool at_least_one_test_was_attack = false;
    for (int i = 0; i < P.last_cusum_test_was_attack.size(); i++)
	if (P.last_cusum_test_was_attack.at(i) == true)
	    at_least_one_test_was_attack = true;

    bool all_tests_were_attacks = true;
    for (int i = 0; i < P.last_cusum_test_was_attack.size(); i++)
	if (P.last_cusum_test_was_attack.at(i) == false)
	    all_tests_were_attacks = false;

    // pause, if at least one metric yielded an alarm
    if ( pause_update_when_attack == 1
	    && at_least_one_test_was_attack == true) {
	logStr.rawPrint(MsgStream::WARN, "  (CUSUM): Pausing update for mean and variance (at least one test was attack)");
	// update values for X
	for (int i = 0; i != P.X_curr.size(); i++) {
	    P.X_curr.at(i) = new_value.at(i);
	}
	return;
    }
    // pause, if all metrics yielded an alarm
    else if ( pause_update_when_attack == 2
	    && all_tests_were_attacks == true) {
	logStr.rawPrint(MsgStream::WARN, "  (CUSUM): Pausing update for mean and variance (all tests were attacks)");
	// update values for X
	for (int i = 0; i != P.X_curr.size(); i++) {
	    P.X_curr.at(i) = new_value.at(i);
	}
	return;
    }
    // pause for those metrics, which yielded an alarm
    // and update only the others
    else if (pause_update_when_attack == 3
	    && at_least_one_test_was_attack == true) {
	logStr.rawPrint(MsgStream::WARN, "  (CUSUM): Pausing update for mean and variance (for those metrics which were attacks)");
	for (int i = 0; i != P.mean.size(); i++) {
	    if (P.last_cusum_test_was_attack.at(i) == false) {
		// update mean and variance with X value from last time
		P.mean.at(i) = P.mean.at(i) * (1 - smoothing_constant) + P.X_curr.at(i) * smoothing_constant;
		if(log_variance_ewma)
		    P.variance.at(i) = exp(log(P.variance.at(i)) * (1 - smoothing_constant) + log((P.X_curr.at(i) - P.mean.at(i))*(P.X_curr.at(i) - P.mean.at(i))) * smoothing_constant);
		else
		    P.variance.at(i) = P.variance.at(i) * (1 - smoothing_constant) + (P.X_curr.at(i) - P.mean.at(i))*(P.X_curr.at(i) - P.mean.at(i)) * smoothing_constant;
	    }
	    // update value for X
	    P.X_curr.at(i) = new_value.at(i);
	}
    }
    // Otherwise update all mean and variance per EWMA
    else {
	logStr.rawPrint(MsgStream::WARN, "  (CUSUM): Update mean and variance for all metrics");
	for (int i = 0; i != P.mean.size(); i++) {
	    // update mean and variance with X value from last time
	    P.mean.at(i) = P.mean.at(i) * (1 - smoothing_constant) + P.X_curr.at(i) * smoothing_constant;
	    if(log_variance_ewma)
		P.variance.at(i) = exp(log(P.variance.at(i)) * (1 - smoothing_constant) + log((P.X_curr.at(i) - P.mean.at(i))*(P.X_curr.at(i) - P.mean.at(i))) * smoothing_constant);
	    else
		P.variance.at(i) = P.variance.at(i) * (1 - smoothing_constant) + (P.X_curr.at(i) - P.mean.at(i))*(P.X_curr.at(i) - P.mean.at(i)) * smoothing_constant;
	    // update value for X
	    P.X_curr.at(i) = new_value.at(i);
	}
    }

    if(logStr.getLogLevel() >= MsgStream::INFO) {
	for (int i = 0; i != P.mean.size(); i++)
	    logStr << MsgStream::raw << MsgStream::INFO << P.mean.at(i) << ", " << sqrt(P.variance.at(i)) << MsgStream::endl;
    }
}

// ------- FUNCTIONS USED TO CONDUCT TESTS ON THE SAMPLES ---------

// statistical test function for wkp-tests
// (optional, depending on how often the user wishes to do it)
void Stat::wkp_test (Params & P) {

    // Containers for the values of single metrics
    std::list<int64_t> sample_old_single_metric;
    std::list<int64_t> sample_new_single_metric;

    std::vector<MetricData>::iterator it = metrics.begin();

    // for every value (represented by *it) in metrics,
    // do the tests
    int index = 0;
    std::string metricstr;

    while (it != metrics.end()) {
	if(use_pca) {
	    std::stringstream tmp;
	    tmp << "pca_comp_" << index;
	    metricstr = tmp.str();
	} else {
	    metricstr = it->name;
	}

	logStr << MsgStream::raw << MsgStream::WARN << "### Performing WKP-Tests for " << metricstr << ":" << MsgStream::endl;

	// storing the last-test flags
	bool tmp_wmw = P.last_wmw_test_was_attack.at(index);
	bool tmp_ks = P.last_ks_test_was_attack.at(index);
	bool tmp_pcs = P.last_pcs_test_was_attack.at(index);

	sample_old_single_metric = getSingleMetric(P.Old, index);
	sample_new_single_metric = getSingleMetric(P.New, index);

	double p_wmw, p_ks, p_pcs;
	p_wmw = p_ks = p_pcs = 1.0;

	// Wilcoxon-Mann-Whitney test:
	if (enable_wmw_test == true && P.wkp_updated) {
	    p_wmw = stat_test_wmw(sample_old_single_metric, sample_new_single_metric, tmp_wmw, metricstr);
	    // New anomaly?
	    if (significance_level > p_wmw && (report_only_first_attack == false || P.last_wmw_test_was_attack.at(index) == false))
		(P.wmw_alarms).at(index)++;
	    P.last_wmw_test_was_attack.at(index) = tmp_wmw;
	}

	// Kolmogorov-Smirnov test:
	if (enable_ks_test == true && P.wkp_updated) {
	    p_ks = stat_test_ks (sample_old_single_metric, sample_new_single_metric, tmp_ks, metricstr);
	    if (significance_level > p_ks && (report_only_first_attack == false || P.last_ks_test_was_attack.at(index) == false))
		(P.ks_alarms).at(index)++;
	    P.last_ks_test_was_attack.at(index) = tmp_ks;
	}

	// Pearson chi-square test:
	if (enable_pcs_test == true && P.wkp_updated) {
	    p_pcs = stat_test_pcs(sample_old_single_metric, sample_new_single_metric, tmp_pcs, metricstr);
	    if (significance_level > p_pcs && (report_only_first_attack == false || P.last_pcs_test_was_attack.at(index) == false))
		(P.pcs_alarms).at(index)++;
	    P.last_pcs_test_was_attack.at(index) = tmp_pcs;
	}

	if (P.wkp_updated == false) {
	    P.last_wmw_test_was_attack.at(index) = false;
	    P.last_ks_test_was_attack.at(index) = false;
	    P.last_pcs_test_was_attack.at(index) = false;
	}

	// generate output files, if wished
	if (createFiles == true) {

	    std::string filename;
	    filename = P.correspondingEndPoint + "." + metricstr + ".wkpparams.txt";

	    chdir(output_dir.c_str());

	    std::ofstream file(filename.c_str(), std::ios_base::app);

	    // are we at the beginning of the file?
	    // if yes, write the param names to the file ...
	    long pos;
	    pos = file.tellp();

	    if (pos == 0) {
		file << "#Value\tp(wmw)\talarms(wmw)\tp(ks)\talarms(ks)\tp(pcs)\talarms(pcs)\tSlevel\tTest-Run\n";
	    }

	    // metric p-value(wmw) #alarms(wmw) p-value(ks) #alarms(ks)
	    // p-value(pcs) #alarms(pcs) counter
	    if(P.wkp_updated) {
		file << sample_new_single_metric.back() << "\t" << p_wmw << "\t"
		    << (P.wmw_alarms).at(index) << "\t" << p_ks << "\t"
		    << (P.ks_alarms).at(index) << "\t" << p_pcs << "\t"
		    << (P.pcs_alarms).at(index) << "\t" << significance_level << "\t"
		    << test_counter << "\n";
	    }
	    else {
		file << "0" << "\t" << p_wmw << "\t"
		    << (P.wmw_alarms).at(index) << "\t" << p_ks << "\t"
		    << (P.ks_alarms).at(index) << "\t" << p_pcs << "\t"
		    << (P.pcs_alarms).at(index) << "\t" << significance_level << "\t"
		    << test_counter << "\n";
	    }

	    file.close();
	    chdir("..");
	}

	it++;
	index++;
    }

    P.wkp_updated = false;
    return;

}

// statistical test function / cusum-test
// (optional, depending on how often the user wishes to do it)
void Stat::cusum_test(Params & P) {

    // we have to store, for which metrics an attack was detected and set
    // the corresponding last_cusum_test_was_attack-flags to true/false
    std::vector<bool> was_attack;
    for (int i = 0; i < metrics.size(); i++)
	was_attack.push_back(false);
    // index, as we cant use the iterator for dereferencing the elements of
    // CusumParams
    int i = 0;
    // current adapted threshold
    double N = 0.0;
    // beta, needed to make (Xn - beta) slightly negative in the mean
    double beta = 0.0;

    std::string metricstr;

    for (std::vector<MetricData>::iterator it = metrics.begin(); it != metrics.end(); it++) {

	if(use_pca) {
	    std::stringstream tmp;
	    tmp << "pca_comp_" << i;
	    metricstr = tmp.str();
	} else {
	    metricstr = it->name;
	}

	logStr << MsgStream::raw << MsgStream::WARN << "### Performing CUSUM-Test for " << metricstr << ":" << MsgStream::endl;

	// Calculate N and beta
	N = repetition_factor * (amplitude_percentage * sqrt(P.variance.at(i)) / 2.0);
	beta = P.mean.at(i) + (amplitude_percentage * sqrt(P.variance.at(i)) / 2.0);

	logStr << MsgStream::raw << MsgStream::DEBUG << " Cusum test returned:\n"
	    << "  Threshold: " << N << "\n"
	    << "  reject H0 (no attack) if current value of statistic g > " << N << MsgStream::endl;

	// TODO
	// "attack still in progress"-message?

	// perform the test and if g > N raise an alarm
	if ( P.cusum_updated && ((P.g.at(i) = cusum(P.X_curr.at(i), beta, P.g.at(i))) > N )) {

	    if (report_only_first_attack == false
		    || P.last_cusum_test_was_attack.at(i) == false) {

		(P.cusum_alarms).at(i)++;

		logStr << MsgStream::raw << MsgStream::FATAL << "cusum: attack for " << metricstr << " detected!" << MsgStream::endl;
		logStr << MsgStream::raw << MsgStream::ERROR
		    << "Test counter: " << test_counter << "\n"
		    << "g = " << P.g.at(i) << "\n"
		    << "N = " << N << MsgStream::endl;
		msgStr << MsgStream::INFO << "cusum: attack for " << metricstr << " detected!" << MsgStream::endl;

#ifdef IDMEF_SUPPORT_ENABLED
		idmefMessage.setAnalyzerAttr("", "", "cusum-test", "");
		sendIdmefMessage("DDoS", idmefMessage);
		idmefMessage = getNewIdmefMessage();
#endif
	    }

	    was_attack.at(i) = true;

	}
	else if (!P.cusum_updated) // reset g to 0 if no new value for that Endpoint occurred
	    P.g.at(i) = 0;

	if (createFiles == true) {

	    std::string filename;
	    filename = P.correspondingEndPoint  + "." + metricstr + ".cusumparams.txt";

	    chdir(output_dir.c_str());

	    std::ofstream file(filename.c_str(), std::ios_base::app);

	    // are we at the beginning of the file?
	    // if yes, write the param names to the file ...
	    long pos;
	    pos = file.tellp();

	    if (pos == 0) {
		file << "#Value\tg\tN\tmean\tstd_dev\tbeta\talarms\tTest-Run\n";
	    }

	    // X  g N mean variance beta #alarms counter
	    if (P.cusum_updated) {
		file << P.X_curr.at(i) << "\t" << P.g.at(i)
		    << "\t" << N << "\t" << P.mean.at(i) << "\t" << sqrt(P.variance.at(i)) << "\t" << beta
		    << "\t" << (P.cusum_alarms).at(i) << "\t" << test_counter << "\n";
	    }
	    else {
		file << "0" << "\t" << P.g.at(i)
		    << "\t" << N << "\t" << P.mean.at(i) << "\t" << sqrt(P.variance.at(i)) << "\t"  << beta
		    << "\t" << (P.cusum_alarms).at(i) << "\t" << test_counter << "\n";
	    }
	    file.close();
	    chdir("..");
	}

	i++;
    }

    if (P.cusum_updated) {
	for (int i = 0; i != was_attack.size(); i++) {
	    if (was_attack.at(i) == true)
		P.last_cusum_test_was_attack.at(i) = true;
	    else
		P.last_cusum_test_was_attack.at(i) = false;
	}
    }
    else {
	for (int i = 0; i != P.last_cusum_test_was_attack.size(); i++)
	    P.last_cusum_test_was_attack.at(i) = false;
    }

    P.cusum_updated = false;
    return;
}

// functions called by the wkp_test()-function
std::list<int64_t> Stat::getSingleMetric(const std::list<std::vector<int64_t> > & l, const short & i) {
    std::list<int64_t> result;
    std::list<std::vector<int64_t> >::const_iterator it = l.begin();
    while ( it != l.end() ) {
	result.push_back(it->at(i));
	it++;
    }
    return result;
}


double Stat::stat_test_wmw (std::list<int64_t> & sample_old,
	std::list<int64_t> & sample_new, bool & last_wmw_test_was_attack, std::string metric) {

    double p;

    if (logStr.getLogLevel() == MsgStream::DEBUG) {
	logStr.rawPrint(MsgStream::DEBUG, " Wilcoxon-Mann-Whitney test details:");
	p = wmw_test(sample_old, sample_new, wmw_two_sided, logStr.getLogfile());
	logStr << MsgStream::raw << MsgStream::DEBUG << " Wilcoxon-Mann-Whitney test returned\n"
	    << "  p-value: " << p << "\n"
	    << "  reject H0 (no attack) to any significance level alpha > " << p << MsgStream::endl;
    }
    else {
	std::ofstream dump("/dev/null");
	p = wmw_test(sample_old, sample_new, wmw_two_sided, dump);
    }


    if (significance_level > p) {
	if (report_only_first_attack == false
		|| last_wmw_test_was_attack == false) {
	    logStr << MsgStream::raw << MsgStream::FATAL << "wmw: attack detected to significance level " << significance_level << " in metric " << metric << "!" << MsgStream::endl;
	    logStr << MsgStream::raw << MsgStream::ERROR 
		<< "Test counter: " << test_counter << "\np-value: " << p << MsgStream::endl;
	    msgStr << MsgStream::INFO << "wmw: attack detected to significance level " << significance_level << " in metric " << metric << "!" << MsgStream::endl;
#ifdef IDMEF_SUPPORT_ENABLED
	    idmefMessage.setAnalyzerAttr("", "", "wmw-test", "");
	    sendIdmefMessage("DDoS", idmefMessage);
	    idmefMessage = getNewIdmefMessage();
#endif
	}
	last_wmw_test_was_attack = true;
    }
    else
	last_wmw_test_was_attack = false;

    return p;
}

double Stat::stat_test_ks (std::list<int64_t> & sample_old,
	std::list<int64_t> & sample_new, bool & last_ks_test_was_attack, std::string metric) {

    double p;

    if (logStr.getLogLevel() == MsgStream::DEBUG) {
	logStr.rawPrint(MsgStream::DEBUG, " Kolmogorov-Smirnov test details:");
	p = ks_test(sample_old, sample_new, logStr.getLogfile());
	logStr << MsgStream::raw << MsgStream::DEBUG << " Kolmogorov-Smirnov test returned\n"
	    << "  p-value: " << p << "\n"
	    << "  reject H0 (no attack) to any significance level alpha > " << p << MsgStream::endl;
    }
    else {
	std::ofstream dump ("/dev/null");
	p = ks_test(sample_old, sample_new, dump);
    }


    if (significance_level > p) {
	if (report_only_first_attack == false
		|| last_ks_test_was_attack == false) {
	    logStr << MsgStream::raw << MsgStream::FATAL << "ks: attack detected to significance level " << significance_level << " in metric " << metric << "!" << MsgStream::endl;
	    logStr << MsgStream::raw << MsgStream::ERROR 
		<< "Test counter: " << test_counter << "\np-value: " << p << MsgStream::endl;
	    msgStr << MsgStream::INFO << "ks: attack detected to significance level " << significance_level << " in metric " << metric << "!" << MsgStream::endl;
#ifdef IDMEF_SUPPORT_ENABLED
	    idmefMessage.setAnalyzerAttr("", "", "ks-test", "");
	    sendIdmefMessage("DDoS", idmefMessage);
	    idmefMessage = getNewIdmefMessage();
#endif
	}
	last_ks_test_was_attack = true;
    }
    else
	last_ks_test_was_attack = false;

    return p;
}


double Stat::stat_test_pcs (std::list<int64_t> & sample_old,
	std::list<int64_t> & sample_new, bool & last_pcs_test_was_attack, std::string metric) {

    double p;

    if (logStr.getLogLevel() == MsgStream::DEBUG) {
	logStr.rawPrint(MsgStream::DEBUG, " Pearson chi-square test details:");
	p = pcs_test(sample_old, sample_new, logStr.getLogfile());
	logStr << MsgStream::raw << MsgStream::DEBUG << " Pearson chi-square test returned\n"
	    << "  p-value: " << p << "\n"
	    << "  reject H0 (no attack) to any significance level alpha > " << p << MsgStream::endl;
    }
    else {
	std::ofstream dump ("/dev/null");
	p = pcs_test(sample_old, sample_new, dump);
    }


    if (significance_level > p) {
	if (report_only_first_attack == false
		|| last_pcs_test_was_attack == false) {
	    logStr << MsgStream::raw << MsgStream::FATAL << "pcs: attack detected to significance level " << significance_level << " in metric " << metric << "!" << MsgStream::endl;
	    logStr << MsgStream::raw << MsgStream::ERROR 
		<< "Test counter: " << test_counter << "\np-value: " << p << MsgStream::endl;
	    logStr << MsgStream::raw << MsgStream::INFO << "pcs: attack detected to significance level " << significance_level << " in metric " << metric << "!" << MsgStream::endl;
#ifdef IDMEF_SUPPORT_ENABLED
	    idmefMessage.setAnalyzerAttr("", "", "pcs-test", "");
	    sendIdmefMessage("DDoS", idmefMessage);
	    idmefMessage = getNewIdmefMessage();
#endif
	}
	last_pcs_test_was_attack = true;
    }
    else
	last_pcs_test_was_attack = false;

    return p;
}

double Stat::covariance (const double & sumProduct, const double & sumX, const double & sumY) {
    // calculate the covariance
    // KOV = 1/N-1 * sum(x1 - n1)(x2 - n2)
    // = 1/N-1 * sum(x1x2 - x1n2 - x2n1 + n1n2)
    // = 1/N-1 * (sum(x1x2) - n2*sum(x1) - n1*sum(x2) + N*n1n2)
    // with n1 = 1/N * sum(x1) and n2 = 1/N * sum(x2)
    // KOV = 1/N-1 * ( sum(x1x2) - 1/N * (sum(x1)sum(x2)) )
    return (sumProduct - (sumX*sumY) / pca_learning_phase) / (pca_learning_phase - 1.0);
}

double Stat::standard_deviation (const double & sumProduct, const double & sumX) {
    // calculate the standard deviation with m = mean value of x
    // SA = sqrt(1/N-1 * (sum((x - m)^2)))
    //    = sqrt(1/N-1 * (sum(x^2 - 2*x*m + m^2)))
    //    = sqrt(1/N-1 * (sum(x^2) - 2*m*sum(x) + sum(m^2)))
    //    = sqrt(1/N-1 * (sum(x^2) - 2*m*sum(x) + N*m^2))
    // with m = 1/N * sum(x)
    //    = sqrt(1/N-1 * (sum(x^2) - 2/N*(sum(x))^2 + 1/N*(sum(x))^2))
    //    = sqrt(1/N-1 * (sum(x^2) - 1/N*(sum(x))^2))
    return sqrt(fabs((sumProduct - (sumX*sumX) / pca_learning_phase) / (pca_learning_phase - 1.0)));
}

void Stat::sigTerm(int signum)
{
    stop();
}

void Stat::sigInt(int signum)
{
    stop();
}
