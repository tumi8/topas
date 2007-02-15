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

#include "metering.h"


#include <stdexcept>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


std::string Metering::directoryName = "";



Metering::Metering(const std::string logname)
{
	outfile.open((directoryName + logname).c_str());
	if (!outfile.is_open()) {
		throw std::runtime_error("Could not open logfile " + logname
					 + " for writing");
	}
}

void Metering::addValue(unsigned i)
{
	if (i == 0 ) {
		outfile << time(NULL) << std::endl;
	} else {
		outfile << i << std::endl;
	}
}

void Metering::setDirectoryName(const std::string& dirname) 
{
        struct stat buf;
        int n;
        if (-1 == (n = lstat(dirname.c_str(), &buf)) && errno != ENOENT) {
                throw std::runtime_error("Could not execute lstat on " + dirname + ": " + strerror(errno));
        }         
        if (errno == ENOENT && n != 0) {
                if (-1 == mkdir(dirname.c_str(), S_IRWXU)) {
                       throw std::runtime_error("Could not create dirname " + dirname + ": " + strerror(errno)); 
                }
        }
	directoryName = dirname;
}
