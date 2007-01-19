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

#include "exceptions.h"

#include <sstream>

using namespace exceptions;

DetectionModuleError::DetectionModuleError(const std::string& who, const std::string& mess, const std::string& expl) 
{
        std::stringstream ss;
        ss << who << ": " << mess << "  Reason: " <<  expl;
        this->s = ss.str();
}


const char* ConfigError::what() const throw() 
{
        return s.c_str();       
}

const char* DetectionModuleError::what() const throw() 
{
        return s.c_str();
}
