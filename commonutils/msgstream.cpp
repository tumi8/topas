/**************************************************************************/
/*    Copyright (C) 2007 Gerhard Muenz                                    */
/*    University of Tuebingen, Germany                                    */
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

#include "msgstream.h"


void MsgStream::setName(const std::string& newname)
{
    name = newname;
}

void MsgStream::setLevel(MsgLevel level)
{
    outputLevel = level;
}

void MsgStream::setLogLevel(MsgLevel level)
{
    logLevel = level;
}

bool MsgStream::openLogfile(const std::string& filename) 
{
    logfile.open(filename.c_str());
    return !logfile.fail();
}

void MsgStream::closeLogfile() 
{
    logfile.close();
}

void MsgStream::printIntro(MsgLevel level)
{
    switch(level){
	case FATAL:
	    std::cout << "FATAL [" << name << "]: ";
	    break;
	case ERROR:
	    std::cout << "ERROR [" << name << "]: ";
	    break;
	case WARN:
	    std::cout << "WARNING [" << name << "]: ";
	    break;
	case INFO:
	    std::cout << "INFORMATION [" << name << "]: ";
	    break;
	case DEBUG:
	    std::cout << "DEBUG [" << name << "]: ";
	    break;
    }
}

void MsgStream::logIntro(MsgLevel level)
{
    switch(level){
	case FATAL:
	    logfile << "FATAL [" << name << "]: ";
	    break;
	case ERROR:
	    logfile << "ERROR [" << name << "]: ";
	    break;
	case WARN:
	    logfile << "WARNING [" << name << "]: ";
	    break;
	case INFO:
	    logfile << "INFORMATION [" << name << "]: ";
	    break;
	case DEBUG:
	    logfile << "DEBUG [" << name << "]: ";
	    break;
    }
}

void MsgStream::print(MsgLevel level, const std::string& msg)
{
  if(level <= outputLevel)
  {
    printIntro(level);
    std::cout << msg << std::endl;
  }
  if(level <= logLevel)
  {
    logIntro(level);
    logfile << msg << std::endl;
  }
}

void MsgStream::rawPrint(MsgLevel level, const std::string& msg)
{
  if(level <= outputLevel)
  {
    std::cout << msg << std::endl;
  }
  if(level <= logLevel)
  {
    logfile << msg << std::endl;
  }
}

