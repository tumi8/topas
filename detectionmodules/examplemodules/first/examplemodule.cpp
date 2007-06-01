/**************************************************************************/
/*    Copyright (C) 2005-2007 Lothar Braun <mail@lobraun.de>              */
/*                            Gerhard Muenz                               */
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

#include "examplemodule.h"


#include <iostream>
#include <fstream>


const char offlineFile[] = "datastore.txt";

ExampleModule::ExampleModule() 
#ifdef OFFLINE_ENABLED
        : DetectionBase<ExampleDataStorage, OfflineInputPolicy<ExampleDataStorage> >()
#else
        : DetectionBase<ExampleDataStorage>()
#endif
{
        init();
}

ExampleModule::ExampleModule(const std::string& configfile)
#ifdef OFFLINE_ENABLED
        : DetectionBase<ExampleDataStorage, OfflineInputPolicy<ExampleDataStorage> >(configfile)
#else
        : DetectionBase<ExampleDataStorage>(configfile)
#endif
{
        init();
}

void ExampleModule::init()
{
	/* signal handlers */
	if (signal(SIGTERM, sigTerm) == SIG_ERR) {
		msg(MSG_ERROR, "Couldn't install signal handler for SIGTERM.\n ");
        } 
	if (signal(SIGINT, sigInt) == SIG_ERR) {
		msg(MSG_ERROR, "Couldn't install signal handler for SIGINT.\n ");
        } 	

#ifdef OFFLINE_ENABLED
	/* open file with offline data */ 
	if(!OfflineInputPolicy<ExampleDataStorage>::openOfflineFile(offlineFile)) {	    
	    msgStr << MsgStream::ERROR << "Could not open offline data file" << MsgStream::endl;
	}
#else
	/* open file to store data for offline use */
	storefile.open(offlineFile);
        /* we want to receive all destination ip address fields */
        subscribeTypeId(IPFIX_TYPEID_destinationIPv4Address);
#endif

#ifdef IDMEF_SUPPORT_ENABLED
	/* register module */
	registerModule("first");
#endif

        /* set alarm time to 10 second.
           That means: The test() methode will be called ten second after
                       the last test()-run ended
        */
#ifdef OFFLINE_ENABLED
        setAlarmTime(0);
#else
        setAlarmTime(10);
#endif

        threshold = 1;

        outfile.open("first.txt");
}

ExampleModule::~ExampleModule() 
{
	outfile.close();
	if(storefile.is_open())
	    storefile.close();
}

#ifdef IDMEF_SUPPORT_ENABLED
void ExampleModule::update(XMLConfObj* xmlObj)
{
	msgStr << MsgStream::INFO << "Update received! ";
	if (xmlObj->nodeExists("stop")) {
		msgStr << "-> stoping module..." << msgStr::endl;
		stop();
	} else if (xmlObj->nodeExists("restart")) {
		msgStr << "-> restarting module..." << msgStr::endl;
		restart();
	} else if (xmlObj->nodeExists("config")) {
		msgStr << "-> updating module configuration..." << msgStr::endl;
	} else { // add your commands here
		msgStr << "-> unknown operation" << msgStr::endl;
	}
}
#endif

void ExampleModule::test(ExampleDataStorage* store) 
{
	msgStr.print(MsgStream::INFO, "Perform TEST");

#ifndef OFFLINE_ENABLED
	/* store data storage for offline use */
	storefile << store;
#endif
    
        /* lets count the collected ip-addresses and compare
           that number to the threshold
        */
        std::map<IpAddress, int> counter;

        // Count how often each ip address was stored
        for (unsigned i = 0; i != store->size(); ++i) {
                counter[(*store)[i]]++;
        }

        
        outfile << std::endl << "Destination addresses found in more than " << threshold
                << " records: " << std::endl;

#ifdef IDMEF_SUPPORT_ENABLED
        IdmefMessage& idmefMessage = getNewIdmefMessage("first", "Example classification");
#endif
        // print ip address and counter value if counter value is greater than threshold
        for (std::map<IpAddress, int>::const_iterator i = counter.begin();
             i != counter.end(); ++i) {
                if (i->second > threshold) {
                        outfile << "Found " << i->first << " in " << i->second 
                                << " records" <<  std::endl;

#ifdef IDMEF_SUPPORT_ENABLED
                        std::string tmp;
                        std::stringstream sstream;
                        sstream << i->second;
                        tmp = sstream.str();
                        idmefMessage.createTargetNode("Don't know what decoy is!!!!", "ipv4-addr",
                                                      i->first.toString(), "0.0.0.0");
                        idmefMessage.createExtStatisticsNode("0", "0", tmp, "0", "0", "0");
                        sendIdmefMessage("Dummy", idmefMessage);
                        idmefMessage = getNewIdmefMessage();
#endif //IDMEF_SUPPORT_ENABLED
                }
        }
        outfile << std::endl;

        /* don't forget to free the store-object */
        delete store;
}

void ExampleModule::sigTerm(int signum)
{
	stop();
}

void ExampleModule::sigInt(int signum)
{
	stop();
}
