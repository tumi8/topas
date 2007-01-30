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

#ifndef _SNORTMODULE_H_
#define _SNORTMODULE_H_

#include "snortstore.h"
#include "configstrings.h"
#include "pcapwriter.h"
#include <detectionbase.h>
#include <commonutils/confobj.h>

#include <string>
#include <fstream>

/**\brief Manages external detecion engine.
* 
* Used to start, end, init the externel detection modul. 
* The test function is called by the collector when a new packet is stored and ready to be passed on to the writer.
*/

class Snortmodule : public DetectionBase<SnortStore, UnbufferedFilesInputPolicy<SemShmNotifier, SnortStore> > {
public:
        /**\brief Default Constructor
	 */
	
	Snortmodule(const std::string& filename);
        
	/**\brief Default Deconstructor
	 */
	
	~Snortmodule();

	/**\brief Returns actuall Pid
	 */
	
	pid_t getPid() const { return pid; }

	/**\brief Called by the collector to process the new arrived packets
	 *
	 */
	
	void test(SnortStore*);

	/**\brief Used to setup the fifos, init the writer, start the external detection modul and send inital file headers
	 *
	 */
	void init();
	
	/**\brief Reads config file 
	 *
	 */

	void readConfig(const std::string& filename);

#ifdef IDMEF_SUPPORT_ENABLED
	/** 
         * Update function. This function will be called, whenever a message
         * for subscribed key is received from xmlBlaster.
         * @param xmlObj Pointer to data structure, containing xml data
         *               You have to delete the memory allocated for the object.
         */
  	void update(XMLConfObj* xmlObj);
#endif
	
	/**\brief Should handle the CleanExit whenever a signal is caught
	 *
	 */
	static void CleanExit();
	
private:
        /**
	 * Config objects
	 */
	
	ConfObj* config;
	std::string execute;
	std::string fifo;
	static const char* FIFO;
	bool calc_thcs;
	bool calc_iphcs;
	std::vector<int> accept_source_ids;	
#ifdef IDMEF_SUPPORT_ENABLED	
	struct wrapperConfig_t {
		bool enable;
		std::string fifoname;
		pthread_t Id;
		void * module;
		std::string topic;
	};

	static wrapperConfig_t wrapperConfig;
#endif	
	/**
	 * Pid and Pipe descriptors
	 */
	
	static pid_t pid;
	static FILE* fifofd;
       

	/**
	* Signal handlers for SIGCHLD
	*/
	
	static void sigChild(int);
	static void sigTerm(int);
	static void sigInt(int);

	/**
	 * Packetwriter
	 */

	static pcapwriter* writer;
	
	/**
	 * stuff 
	 */

	static bool shutdown;

#ifdef IDMEF_SUPPORT_ENABLED
	
	/**
	 * Entrypoint for xmlWrapper thread
	 */

	static void * xmlWrapperEntry(void *pipename);
#endif

};

#endif
