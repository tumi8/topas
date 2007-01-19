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

#ifndef _METERING_H_
#define _METERING_H_


#include <string>
#include <vector>
#include <map>
#include <fstream>


/**
 * This class is for analysing the work of the collector and the detection
 * modules. Because of the fact that modules and collector work in different
 * address spaces, it is necessary to exchange data when evaluating 
 * the collected data.
 * This is done via files.
 */
class Metering {
public:
	Metering(const std::string logname);
	

	/**
	 * Sets directory which is used to store the statistics. The directory
         * will be created if it doesn't exists.
         * @param dirname Directory to write statitistic files
	 */
	static void setDirectoryName(const std::string& dirname);


	void addValue(unsigned v = 0);

private: 
	Metering* instance;

	static std::string directoryName;

	std::ofstream outfile;
};

#endif
