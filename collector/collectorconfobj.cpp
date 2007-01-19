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

#include "collectorconfobj.h"


#include <concentrator/msg.h>


CollectorConfObj::CollectorConfObj(const std::string& file) 
        : XMLConfObj(file)
{
        //checkConfig();
}

CollectorConfObj::~CollectorConfObj() 
{

}

void CollectorConfObj::checkConfig()
{
	msg(MSG_INFO, "Checking configuration...");
	/* TODO: implement this */
	msg(MSG_INFO, "Configuration successfully checked!");
}


void CollectorConfObj::checkValue(const std::string& section, const std::string &entry) 
{
	/* TODO: implement this */
}
