/**************************************************************************/
/*   Copyright (C) 2006-2007 Nico Weber                                   */
/*                                                                        */
/*   This library is free software; you can redistribute it and/or        */
/*   modify it under the terms of the GNU Lesser General Public           */
/*   License as published by the Free Software Foundation; either         */
/*   version 2.1 of the License, or (at your option) any later version.   */
/*                                                                        */
/*   This library is distributed in the hope that it will be useful,      */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*   Lesser General Public License for more details.                      */
/*                                                                        */
/*   You should have received a copy of the GNU Lesser General Public     */
/*   License along with this library; if not, write to the Free Software  */
/*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,           */
/*   MA  02110-1301, USA                                                  */
/**************************************************************************/

/**\file main.cpp
 *
 * Main file for module startup
 */

#include "snortmodule.h"
#include <concentrator/msg.h>
//static void sigIntMain(int signum);
//static void sigTermMain(int signum);
//static Snortmodule* snort;
/**\brief Used to fire up the real module and catch the exceptions.
 *
 * Reads the config file and starts the module
 */

int main(int argc, char **argv)
{
	
	Snortmodule* snort = new Snortmodule(argv[1]);
	//Sighandlers          
	
/*	if (signal(SIGTERM, sigTermMain) == SIG_ERR) {
		msg(MSG_ERROR, "Snortmodule(Main): Couldn't install signal handler for SIGTERM.\n ");
	}
	if (signal(SIGINT, sigIntMain) == SIG_ERR) {
		msg(MSG_ERROR, "Snortmodule(Main): Couldn't install signal handler for SIGINT.\n ");
	}
*/	
	try {
                if (argc != 2) {
                        msg(MSG_ERROR, "No config file specified. Taking default values");
                        snort->readConfig("");
                } else {
                        snort->readConfig(argv[1]);
                }
		snort->init();
		int ret = snort->exec();
		delete snort;
		return ret;
	}catch (std::exception e) {
                msg(MSG_FATAL, "Got unhandled exception: %s", e.what());
                return 0;
        }
}
