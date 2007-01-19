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

#ifndef _COLLECTOR_CONF_OBJ_H_
#define _COLLECTOR_CONF_OBJ_H_


#include <commonutils/confobj.h>


#include <string>


/**
 * Parses XML-files, stores the parsed values and does a consistency check.
 */
class CollectorConfObj : public XMLConfObj {
 public:
        /** 
         * Constructor...
         * @param file XML-file which should be parsed.
         */
        CollectorConfObj(const std::string& file);

        /**
         * Destructor....
         */
        ~CollectorConfObj();

 private:
        /**
         * Checks if value makes sense in combination with section and entry.
         * Excecption exceptions::config_error will be thrown, if stupid value is found.
         * @param section Name of section in ini-file
         * @param entry Name of entry in ini-file
         * @param value Value assigned to entry in ini-file
         */
        void checkValue(const std::string& section, const std::string& entry);

        /**
         * Checks if all necessary entries are available and if the combination of all enties makes sens
         */
        void checkConfig();
};

#endif 
