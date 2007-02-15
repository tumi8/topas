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

#include "examplemodule.h"

#include <iostream>

ExampleModule::ExampleModule() 
        : DetectionBase<ExampleDataStorage>()
{
        init();
}

ExampleModule::ExampleModule(const std::string& configfile)
        : DetectionBase<ExampleDataStorage>(configfile)
{
        init();
}

ExampleModule::~ExampleModule() {
        
}


void ExampleModule::init()
{
        subscribeTypeId(IPFIX_TYPEID_destinationIPv4Address);
        subscribeTypeId(IPFIX_TYPEID_destinationTransportPort);
        subscribeTypeId(IPFIX_TYPEID_packetDeltaCount);

#ifdef IDMEF_SUPPORT_ENABLED
	/* register module */
	registerModule("second");
#endif
        /* set alarm time to 10 second.
           That means: The test() methode will be called ten second after
                       the last test()-run ended
        */
        setAlarmTime(10);
        
        threshold = 10;

        outfile.open("second.txt");

}

#ifdef IDMEF_SUPPORT_ENABLED
void ExampleModule::update(XMLConfObj* xmlObj)
{
	std::cout << "Update received!" << std::endl;
	if (xmlObj->nodeExists("stop")) {
		std::cout << "-> stoping module..." << std::endl;
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

void ExampleModule::test(ExampleDataStorage* store) 
{
        /*
          It's not necessary to count ip addresses in this example, because
          this was already done in ExampleDataStorage::addFieldData()
        */

#ifdef IDMEF_SUPPORT_ENABLED
        IdmefMessage& idmefMessage = getNewIdmefMessage("Second example module", "Example classification");
#endif
        outfile << "******************** Test started *********************" << std::endl;
        for (ExampleDataStorage::ExampleData::const_iterator i = store->begin(); i != store->end(); ++i) {
                if (i->second > threshold) {
                        outfile << "Got " << i->second << " packets to destination "
                                << IpAddress(i->first.data)
                                << "/" << (int)(i->first.data[4]) << " and port "
                                << *reinterpret_cast<const uint16_t*>(&i->first.data[5]) << std::endl;
#ifdef IDMEF_SUPPORT_ENABLED
                        idmefMessage.createTargetNode("Don't know what decoy is!!!!!", "ipv4-addr",
                                                      IpAddress(i->first.data).toString(), "0.0.0.0");
                        std::string tmp;
                        std::stringstream sstream;
                        sstream << i->second;
                        tmp = sstream.str();
                        idmefMessage.createExtStatisticsNode("0", tmp, "0", "0");
                        sendIdmefMessage("Dummy", idmefMessage);
                        idmefMessage = getNewIdmefMessage();
#endif          
                }
        }
        outfile << "********************  Test ended  *********************" << std::endl;
        
        delete store;
}
