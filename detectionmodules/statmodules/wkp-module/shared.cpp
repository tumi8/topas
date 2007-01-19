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

#include "shared.h"

std::ostream & operator << (std::ostream & os, const std::list<int64_t> & L) {
  std::list<int64_t>::const_iterator it = L.begin();
  while (it != L.end()) {
    os << *it << ',';
    it++;
  }
  return os;
}

std::ostream & operator << (std::ostream & os, const std::vector<unsigned> & V) {
  std::vector<unsigned>::const_iterator it = V.begin();
  while (it != V.end()) {
    os << *it << ',';
    it++;
  }
  return os;
}
