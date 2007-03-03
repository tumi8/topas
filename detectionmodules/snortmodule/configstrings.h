#ifndef _CONFIG_STRINGS_H_
#define _CONFIG_STRINGS_H_


#include <string>


namespace ConfigStrings
{
        static const std::string SNORTSECTION    = "snortmodule";
        static const std::string FIFOc            = "fifo";
        static const std::string EXECUTE       	 = "execute";
        static const std::string DEFAULT_FIFO    = "/tmp/snortfifo";
	static const std::string RULE_FILE = "rule_file";
	static const std::string DEFAULT_RULE_FILE = "/etc/snort/rules/own.rules";
	static const std::string CONFIG_FILE = "config_file";
	static const std::string DEFAULT_CONFIG_FILE = "/etc/snort/snort.conf";
        static const std::string DEFAULT_EXECUTE = "/usr/bin/snort -r /tmp/snortfifo -l /tmp/ -L snortlogpcap -v";
	static const std::string DEBUG_LEVEL     = "debug_level";
	static const std::string CALC_THCS = "calculate_transportheader_checksum";
	static const std::string CALC_IPHCS = "calculate_ipheader_checksum";
	static const std::string ACCEPT_SOURCE_IDS = "accept_source_ids";
	static const std::string WRAPPERSECTION = "xmlwrapper";
	static const std::string ENABLE = "enable";
	static const bool 	 DEFAULTENABLE = true;
	static const std::string WRAPPERPIPE = "fifo";
	static const std::string DEFAULTWRAPPERPIPE = "/tmp/xmlWrapper_fifo";
	static const std::string TOPIC = "topic";
	static const std::string DEFAULTTOPIC = "snortmodule";
	static const std::string ANALYZERID = "analyzerid=";
	static const std::string MANUFACTURER = "manufacturer=";
	static const std::string GET_SNORT_CONFIG = "getSnortConfig";
	static const std::string UPDATE_SNORT_CONFIG = "updateSnortConfig";
	static const std::string GET_SNORT_RULE_FILE = "getSnortRuleFile";
	static const std::string UPDATE_SNORT_RULE_FILE = "updateSnortRuleFile";
	static const std::string APPEND_SNORT_RULE_FILE = "appendSnortRuleFile";
	static const std::string CLEAR_SNORT_RULE_FILE = "clearSnortRuleFile";

};

#endif

