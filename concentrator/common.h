/*
IPFIX Collector module
Copyright (C) 2004 Christoph Sommer
http://www.deltadevelopment.de/users/christoph/ipfix

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation; either version 2.1 of the License,
or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
*/


#ifndef TOOLS_H
#define TOOLS_H

#include <stdio.h>
#include <stdint.h>
#include "ipfix.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
#define debugf(message, ...) fprintf(stderr, "[DEBUG] %s l.%d: " message "\n", \
        __FILE__, __LINE__, __VA_ARGS__)

#define infof(message, ...) fprintf(stderr, "[INFO] %s l.%d: " message "\n", \
        __FILE__, __LINE__, __VA_ARGS__)

#define errorf(message, ...) fprintf(stderr, "[ERROR] %s l.%d: " message "\n", \
        __FILE__, __LINE__, __VA_ARGS__)

#define fatalf(message, ...) fprintf(stderr, "[FATAL] %s l.%d: " message "\n", \
        __FILE__, __LINE__, __VA_ARGS__)

#define debug(message) fprintf(stderr, "[DEBUG] %s l.%d: " message "\n", \
        __FILE__, __LINE__)

#define info(message) fprintf(stderr, "[INFO] %s l.%d: " message "\n", \
        __FILE__, __LINE__)

#define error(message) fprintf(stderr, "[ERROR] %s l.%d: " message "\n", \
        __FILE__, __LINE__)

#define fatal(message) fprintf(stderr, "[FATAL] %s l.%d: " message "\n", \
        __FILE__, __LINE__)
*/

uint64_t ntohll(uint64_t i);
uint64_t htonll(uint64_t i);


#ifdef __cplusplus
}
#endif


#endif
