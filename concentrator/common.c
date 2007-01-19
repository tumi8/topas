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



/** \file
 * Generic constants, data types and functions.
 *
 * This module provides stuff useful throughout the application.
 *
 */

#include <netinet/in.h>
#include "common.h"

/**
 * Converts a uint64_t from network byte order to host byte order
 */
uint64_t ntohll(uint64_t i) {
        union {
                uint64_t ui64;
                struct {
                        uint32_t a;
                        uint32_t b;
                } ui32;
        } u;
        u.ui64 = i;
        uint64_t word0 = ntohl(u.ui32.a);
        uint64_t word1 = ntohl(u.ui32.b);
        uint64_t reslt = (word0 << 32) ^ (word1 << 0);
        return reslt;
}

/**
 * Converts a uint64_t from host byte order to network byte order
 */
uint64_t htonll(uint64_t i) {
        union {
                uint64_t ui64;
                struct {
                        uint32_t a;
                        uint32_t b;
                } ui32;
                } u;
        u.ui32.a = htonl(i >> 32);
        u.ui32.b = htonl(i >> 0);
        return u.ui64;
}
