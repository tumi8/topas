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

/**
 * @author Gerhard Muenz <muenz@informatik.uni-tuebingen.de>
 */

#include "countmodule.h"

#include <iostream>
#include <cstdlib>
#include <commonutils/msgstream.h>

MsgStream ms(MsgStream::WARN, "CountModule");

/* demonstrates the use of libdetectionModule */
int main(int argc, char** argv) 
{
    if (argc == 2) {
	CountModule m(argv[1]);
	return m.exec();
    }

    ms.print(MsgStream::ERROR, "Configuration file argument is missing!");
    exit(-1);
}
