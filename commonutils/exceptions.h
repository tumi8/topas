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

#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include <exception>
#include <stdexcept>
#include <string>

namespace exceptions {

        /**
         * Exception thrown during errors in the configuration-process
         */
        class ConfigError : public std::exception 
        {
        public:
                ConfigError(const std::string& s) { this->s = s; }
                virtual ~ConfigError() throw() {};
                virtual const char* what() const throw();
        
        private:
                std::string s;
        };
        
        /** 
         * Exception thrown, during configuration of the detection modules and on errors while running the detection modules.
         */ 
        class DetectionModuleError : public std::exception 
        {
        public:
                DetectionModuleError(const std::string& who, const std::string& mess, const std::string& expl);
                DetectionModuleError(const std::string& message) { s = message; }
                virtual ~DetectionModuleError() throw() {};
                virtual const char* what() const throw();
        private:
                std::string s;
        };

	/**
	 * Thrown to signal errors on XML parsing process.
	 */
	typedef ConfigError XMLException;
};


#endif
