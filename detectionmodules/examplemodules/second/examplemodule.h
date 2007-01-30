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

#ifndef _EXAMPLEMODULE_H_
#define _EXAMPLEMODULE_H_


#include "exampledatastorage.h"


#include <detectionbase.h>


#include <fstream>


class ExampleModule : public DetectionBase<ExampleDataStorage> 
{
 public:
        ExampleModule();
        ExampleModule(const std::string& exampleModule);
        ~ExampleModule();

        /**
         * Test function. This function will be called to do the tests.
         * @param store Pointer to an ExampleDataStorage containing all
         *              data collected since the last call to test.
                        This pointer points to your memory. That means,
                        that your may store the pointer.
                        IMPORTANT: _YOU_ have to free the objects memory
                        after you don't need it any longer (use delete-operator
                        to do this)
         */
        void test(ExampleDataStorage* store);

#ifdef IDMEF_SUPPORT_ENABLED
	/** 
         * Update function. This function will be called, whenever a message
         * for subscribed key is received from xmlBlaster.
         * @param xmlObj Pointer to data structure, containing xml data
         *               You have to delete the memory allocated for the object.
         */
  	void update(XMLConfObj* xmlObj);
#endif

private:
        int threshold;

        std::ofstream outfile;

        void init();
};


#endif
