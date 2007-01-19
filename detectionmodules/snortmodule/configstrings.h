#ifndef _CONFIG_STRINGS_H_
#define _CONFIG_STRINGS_H_


#include <string>


namespace ConfigStrings
{
        static const std::string SNORTSECTION    = "snortmodule";
        static const std::string FIFOc            = "fifo";
        static const std::string EXECUTE       	 = "execute";
        static const std::string DEFAULT_FIFO    = "/tmp/snortfifo";
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
	static const std::string TOPIC ="topic";
	static const std::string DEFAULTTOPIC ="snortmodule";
};

#endif

