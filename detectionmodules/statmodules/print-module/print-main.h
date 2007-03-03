/**************************************************************************/
/*    Copyright (C) 2006 Romain Michalec                                  */
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

#ifndef _PRINT_MAIN_H_
#define _PRINT_MAIN_H_

#include<signal.h>

#include "print-store.h"

#include <detectionbase.h>
#include <iostream>
#include <fstream>
#include <sstream>

class Print : public DetectionBase< PrintStore,
	      UnbufferedFilesInputPolicy<SemShmNotifier, PrintStore> > {

 public:

  Print(const std::string & configfile);
  ~Print() {}

  /**
   * Test function. This function will be called in periodic intervals.
   * Set interval size with @c setAlarmTime().
   * @param store Pointer to a PrintStore containing all
   *              data collected since the last call to test.
   *              This pointer points to your memory. That means,
   *              that your may store the pointer.
   *              IMPORTANT: _YOU_ have to free the objects memory
   *              after you don't need it any longer (use delete-operator
   *              to do this)
   */
  void test(PrintStore * store);

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

  std::ofstream outfile;
	
	/**
	 * Signal handlers
	 */
	static void sigTerm(int);
	static void sigInt(int);

  void init(const std::string & configfile);

};

#endif
