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

#include "collector.h"


#include <commonutils/global.h>
#include <commonutils/exceptions.h>
#include <concentrator/msg.h>
#include <concentrator/rcvIpfix.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>


#include <iostream>
#include <cstring>
#include <cstdlib>


/**
 * Prints the usage message
 * @param name Name of the program
 */
void usage(char* name) 
{
        std::cout << "Usage: " << name << " [-h] [-d[[d]d] [-f config_file ]" << std::endl << std::endl;
        std::cout << "Options: " << std::endl;
        std::cout << "-d: will increase the debug level" << std::endl;
        std::cout << "-f config_file: sets the configuration file" << std::endl;
        std::cout << "-h print this help and exit" << std::endl;
}


/**
 * Function that inits and runs the collector
 * @param config_file Destination of the collectors configuration file
 */
int run_collector(const std::string& configFile) 
{
        Collector collector;

        /* read the configuration */
        try {
                collector.readConfig(configFile);
        }catch (exceptions::ConfigError e) {
                msg(MSG_FATAL, "Couldn't configure collector: %s", e.what());
                return error_states::CONFIG_ERROR;
        }

        /* start the detection modules */
        try {
                collector.startModules();
        }catch (exceptions::DetectionModuleError e) {
                msg(MSG_FATAL, "Couldn't start detection modules: %s", e.what());
                return error_states::DETECTION_MODULE_ERROR;
        }
        
        /* wait for detection modules to do their initialisation work */
        msg(MSG_INFO, "Waiting for detection modules to init their stuff....");
        sleep(2);
        msg(MSG_INFO, "Starting up collector...");

        try {
                collector.run();
        } catch (std::exception e) {
                msg(MSG_FATAL, "Error while running the collector: %s", e.what());
                return error_states::RUN_ERROR;
        }

	msg(MSG_DEBUG, "Leaving run_collector()");
        return 0;
}

/**
 * Main function
 * @param argc Number of arguments passed to the collector
 * @param argv Array containing the arguments passed to the collector
 */
int main(int argc, char** argv) 
{
        char option;
        int debug_level = MSG_DEFAULT;
        std::string config_file = config_space::path_to_config + config_space::collector_config;
        opterr = 0;

        /* Parse Program arguments */
        while (EOF != (option = getopt(argc, argv, "-df:"))) {
                switch(option) {
                case 'd':
                        ++debug_level;
                        break;

                case 'f':
                        config_file = optarg;
                        break;
                case 'h':
                default:
                        usage(argv[0]);
                        return error_states::INIT_ERROR;
                }
        }

        msg_setlevel(debug_level);

        /* detach from terminal when debug_level == 0 */
        // TODO: enable detatching      
        debug_level++;
        
        if (debug_level == 0) {
                pid_t pid;
                if (-1 == (pid = fork())) {
                        msg(MSG_FATAL, "Could not fork a new process: %s", strerror(errno));
                        return error_states::RUN_ERROR;
                } else if (pid == 0) {
                        /* child process */
                        setsid();
                        chdir("/"); /* TODO: working directory should not be /, but working_dir from config file */
                        umask(0);

                        /* detach from terminal */
                        if (EOF == fclose(stdin)) {
                                msg(MSG_FATAL, "Could not close stdin.");
                                return error_states::RUN_ERROR;
                        }
                        if (EOF == fclose(stdout)) {
                                msg(MSG_FATAL, "Could not close stdout.");
                                return error_states::RUN_ERROR;
                        }
                        if (EOF == fclose(stderr)) {
                                msg(MSG_FATAL, "Could not close stderr.");
                                return error_states::RUN_ERROR;
                        }

                        return run_collector(config_file);
                }
                /* parent process */
                /* do nothing */
                std::cout << "Elvis has just left the building" << std::endl;
                return 0;
        }
        else {
                return run_collector(config_file);
        }

        return 0;
}
