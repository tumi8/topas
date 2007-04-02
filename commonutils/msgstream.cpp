/**************************************************************************/
/*    Copyright (C) 2007 Gerhard Muenz                                    */
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



void MsgStream::setLevel(MsgLevel level)
{
    outputLevel = level;
}

void MsgStream::setName(const std::string& newname)
{
    name = newname;
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
	    std::cout << "WARN  [" << name << "]: ";
	    break;
	case INFO:
	    std::cout << "INFO  [" << name << "]: ";
	    break;
	case DEBUG:
	    std::cout << "DEBUG [" << name << "]: ";
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
}

