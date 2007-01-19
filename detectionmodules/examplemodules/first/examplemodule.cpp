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
#include <fstream>


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

void ExampleModule::init()
{
        /* we want to receive all destination ip address fields */
        subscribeTypeId(IPFIX_TYPEID_destinationIPv4Address);

        /* set alarm time to 10 second.
           That means: The test() methode will be called ten second after
                       the last test()-run ended
        */
        setAlarmTime(10);

        threshold = 10;

        outfile.open("first.txt");
}

ExampleModule::~ExampleModule() 
{
        
}


void ExampleModule::test(ExampleDataStorage* store) 
{
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
        IdmefMessage& idmefMessage = getNewIdmefMessage("First example module", "Example classification");
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
                        idmefMessage.createExtStatisticsNode("0", tmp, "0", "0");
                        sendIdmefMessage("Dummy", idmefMessage);
                        idmefMessage = getNewIdmefMessage();
#endif //IDMEF_SUPPORT_ENABLED
                }
        }
        outfile << std::endl;

        /* don't forget to free the store-object */
        delete store;
}
