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
        : DetectionBase<ExampleDataStorage, UnbufferedFilesInputPolicy<SemShmNotifier, ExampleDataStorage> >()
{
        init();
}

ExampleModule::ExampleModule(const std::string& configfile)
        : DetectionBase<ExampleDataStorage, UnbufferedFilesInputPolicy<SemShmNotifier, ExampleDataStorage> >(configfile)
{
        init();
}

void ExampleModule::init()
{
        /* we want to receive all destination ip address fields */
        subscribeTypeId(IPFIX_TYPEID_destinationIPv4Address);
	subscribeTypeId(IPFIX_TYPEID_sourceIPv4Address);
	subscribeTypeId(IPFIX_TYPEID_sourceTransportPort);
	subscribeTypeId(IPFIX_TYPEID_destinationTransportPort);

        /* set alarm time to 10 second.
           That means: The test() methode will be called ten second after
                       the last test()-run ended
        */
        setAlarmTime(0);

        threshold = 10;

	std::cout << "Source IP      |      Destination IP |  Source Port | Destination Port" << std::endl
		  << "-------------------------------------|--------------|-----------------" << std::endl;
}

ExampleModule::~ExampleModule() 
{
        
}


void ExampleModule::test(ExampleDataStorage* store) 
{
	/*
	outfile << "ExampleModule::test(ExampleDataStorage* store) started" << std::endl;
	outfile << store->getIP() << std::endl;
	outfile << "ExmapleModule::test(ExampleDataStorage* store) ended" << std::endl;
	*/
       
	int len = store->getSourceIP().toString().length();
	std::cout << store->getSourceIP().toString();
	for (int i = 0; i != 15 - len; ++i) {
		std::cout << " ";
	}
	std::cout << "| " << store->getDestinationIP().toString();
        
        len = store->getDestinationIP().toString().length();
        for (int i = 0; i != 15 - len; ++i) {
                std::cout << " ";
        }

	std::cout << "| " << ntohs(store->sourcePort)  << " | " << ntohs(store->destinationPort) << std::endl;

        /* don't forget to free the store-object */
        delete store;
}
