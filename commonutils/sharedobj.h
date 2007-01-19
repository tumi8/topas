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

#ifndef _SHARED_OBJS_H_
#define _SHARED_OBJS_H_

#include "global.h"


#include <ostream>


namespace shared 
{
        /**
         * Encapsulates an unsigned integer and ensures, that the integer is always in range [ 0, config_space::MAX_FILES )
         */
        class FileCounter 
        {
        public:
                FileCounter(unsigned c = 0);
                ~FileCounter();
                
                // pre-increment
                const FileCounter& operator++();
                // post-increment 
                const FileCounter operator++(int);
                // pre-decrement
                const FileCounter& operator--();
                // post-decrement
                const FileCounter operator--(int);
                
                FileCounter operator+(const FileCounter& rhs) const;
                
                FileCounter operator+(const unsigned& rhs) const;
                
                FileCounter operator-(const FileCounter& rhs) const;
                
                FileCounter operator-(const unsigned& rhs) const;

                FileCounter& operator=(const unsigned&);
                FileCounter& operator=(const FileCounter&);
                
                bool operator==(const FileCounter& rhs) const;

                bool operator!=(const FileCounter& rhs);
                bool operator!=(const unsigned &i) const;

                operator unsigned () const;

        private:
                friend std::ostream& operator<<(std::ostream& os, const FileCounter& fc);
                unsigned i;
        };



        /** 
         * Encapsulates a shared memory block. The shared memory block is used to exchange information
         * about incoming IPFIX packets between collector and assigned detection modules.
         * This class does NOT synchronise access to the shared memory block.
         */
        class SharedObj 
        {
        public:
                /**
                 * Tries to allocate a new shared memory block (config_space::SHM_SIZE bytes).
                 */
                SharedObj();

                /**
                 * Tries to alloate an existing shared memory block.
                 * @param s Key for the shared memory block
                 */
                SharedObj(key_t s);

                /**
                 * Destructor...
                 */
                ~SharedObj();

                /**
                 * Returns content of field "from" from the shared memory block
                 * @return @c file_counter representing the unsigned integer value stored 
                 *         in field "from" within the shared memory block
                 */
                FileCounter from() const { return FileCounter(sb->from); }

                /**
                 * Returns content of field "to" from the shared memory block
                 * @return @c file_counter representing the unsinged integer value stored
                 *         in field "to" wihtin the shared memory block
                 */
                FileCounter to() const { return FileCounter(sb->to); }

                /**
                 * Sets unsinged interger value in field "from" within the shared memory block
                 * @param f value to set the field "from" to
                 */
                void setFrom(const FileCounter& f) { sb->from = (unsigned)f; }

                /**
                 * Sets unsigned integer value in field "to" within the shared memory block
                 * @param t value to set the field "to" to
                 */
                void setTo(const FileCounter& t) { sb->to = (unsigned)t; }
                
                /**
                 * Returns key to the allocated shared memory block
                 * @return key to the allocated shared memory block
                 */
                key_t getShmKey() const { return shmKey; }

                /**
                 * Returns name of subdirectory the files are stored in.
                 * The name is a number (precise: sourceID of monitor sending
                 * the IPFIX data)
                 * return SourceID of monitor sending IPFIX data. This is also the
                 *        subdirectories name in which the IPFIX Files can be found
                 */
                uint32_t getSourceID() 
                {
                        return sb->sourceID;
                }

                /**
                 * Sets subdirectory name (SourceId of sending monitor) in which the
                 * IPFIXFiles are stored
                 * @param sourceID Source id of sending monitor
                 */
                void setSourceID(uint32_t sourceID) 
                {
                        sb->sourceID = sourceID;
                }

		/**
		 * Sets storage shared memory id
		 * @param key shared memory key of the storage area
		 */
		void setStorageKey(key_t key)
		{
			sb->storageKey = key;
		}

		/**
		 * Gets the storage shared memory id
		 * @return shared memory id
		 */
		key_t getStorageKey() { return sb->storageKey; }

		/**
		 * Sets shared memory storage size.
		 * @param size size
		 */
		void setStorageSize(size_t size) {
			sb->storageSize = size;
		}

		size_t getStorageSize() {
			return sb->storageSize;
		}

        private:
                /** 
                 * Hidden (unimplemented) copy constructor
                 */
                SharedObj(const SharedObj&){}
                /**
                 * Hidden (unimplemented) copy constructor
                 */
                SharedObj(SharedObj&) {}

                key_t shmKey;
                int shmId;

                /* this struct will be out into the shared memory block */
                typedef struct 
                {
                        uint32_t sourceID;
                        unsigned from;
                        unsigned to;
			key_t storageKey;
			unsigned storageSize;
                } ShmBlock;

                /**
                 * Pointer to the shared memory block.
                 */
                ShmBlock* sb;

		bool deleteBlock;
        };

};

#endif
