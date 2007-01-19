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

#ifndef PACKETSTATS_H
#define PACKETSTATS_H

/**
 * @author Lothar Braun <braunl@informatik.uni-tuebingen.de>
 */


#include "global.h"
#include "sharedobj.h"
#include "mutex.h"


#include <concentrator/msg.h>


#include <unistd.h>
#include <stdio.h>
#include <errno.h>


#include <list>
#include <cctype>
#include <fstream>
#include <string>
#include <stdexcept>


/**
 * 
 */
struct PacketStats 
{
	shared::FileCounter newest;
        shared::FileCounter oldest;
};

/**
 * Base class for IPFIXPacket stroring facillities.
 */
class PacketStorage {
public:
        uint16_t readPacket(byte** d) { return 0; }
        virtual void proceedOnePacket() = 0;
protected:
        PacketStorage() {}
};


/**
 * Handles incoming IPFIX-Packets from the time they arrive, by writing them onto a
 * file system. The files will be removed by the collector after they where processed
 */
class IpfixFile : public PacketStorage
{
public:
        static IpfixFile* writePacket(const char* filename, const byte* data, uint16_t length);
        
        virtual void proceedOnePacket();
private:
        /**
         * Hidden Copy Constructor. These Objects may not be copied.
         */
        IpfixFile(const IpfixFile&);

	/**
	 * Hidden Default Constructor. There is no default ...
	 */
	IpfixFile() { }

        static std::list<std::string> fileNames;

        static IpfixFile* ipfixFile;

        static Mutex listLock;
};


/**
 * Handles incoming IPFIX-Packets from the time they arrive by writing them onto
 * a shared memory storage block.
 */ 
class IpfixShm : public PacketStorage {
public:

	static void setShmPointer(byte*);
	static void setShmSize(size_t);

	static uint16_t readPacket(byte** d);
        virtual void proceedOnePacket();
        static IpfixShm* writePacket(const byte* d, uint16_t len);


private:
        IpfixShm();
        static IpfixShm* instance;

	/** TODO: HACK FOR THE MODULE SITE! */
	static uint16_t packetSize;
	
	/**
	 * location on which this instance of IpfixShm stored it's
	 * packet
	 */
	//void* packetLocation;
	/**
	 * start of the shared memory block
	 */
	static byte* startLocation;

	/**
	 * the next incoming paket will be written to this location
	 */
	static byte* writePosition;

	/**
	 * pointer to the first not yet  processed ipfix packet
	 */
	static byte* readPosition;
	
	/**
	 * total size of the shared memory block
	 */ 
	static size_t size;

	/**
	 * if writeBeforeRead is true, then writePosition < readPosition
	 * else writePosition >= readPosition;
	 */
	static bool writeBeforeRead;
};

/**
 * Stores all IpfixFiles stored within the collector. It 
 * synchornises access between manager and collector.
 */
class IpfixPacketStore 
{
public:
        /**
         * Constructor
         */
        IpfixPacketStore() 
        {
        }

        /**
         * Destructor
         */
        ~IpfixPacketStore() 
        {
        }

        /** 
         * Adds pointer to an IpfixFile object to the queue specified by sourceID
         * @param sourceID Queue to add ipfixFile to
         * @param ipfixPacket IpfixPacket to be stored
         * @return void
         */
        inline void pushIpfixPacket(uint32_t sourceID, PacketStorage* ipfixPacket) 
        {
		mutex.lock();
                files.push_back(ipfixPacket);
                ++fs.newest;
		mutex.unlock();
        }

        /**
         * Deletes the first object from the Queue specified by sourceID.
         * The BeforeDeletion policy is run on the IpfixFile object before it is deleted
         * @param sourceID Queue the first object should be deleted from
         */
        inline void popIpfixPacket(uint32_t sourceID) 
        {
		mutex.lock();
                (*files.begin())->proceedOnePacket();
                files.pop_front();
                ++fs.oldest;
		mutex.unlock();
        }

        /**
         * Deletes the first count IpfixFile objects from the queue specified by sourceID.
         * @param sourceID Queue the first count objects should be deleted from.
         * @param count Number of IpfixFile objects to be deleted.
         */
        void popIpfixPacket(uint32_t sourceID, unsigned count) 
        {
                if (count > files.size()) {
                        throw std::runtime_error("IpfixFileStore: Too many files specified for"
                                                 " deletion");
                }

                for (unsigned i = 0; i != count; ++i) {
                        popIpfixPacket(sourceID);
                }
        }


        /**
         * Checks which queue contains most IpfixFile objects.
         * @return Handle on queue containing most IpfixFile objects
         */
        uint32_t getLongestQueue() 
        {
                return 0;
        }


        /**
         * Returns the @c fileStats object associated with the queue specified by sourceID
         * @param sourceID Specifies the queue the fileStats object should be picked from
         * @return fileStats object
         */
        const PacketStats getPacketStats(uint32_t sourceID) 
        {
                return fs;
        }

private:
        std::list<PacketStorage*> files;

	Mutex mutex;
        PacketStats fs;
};


#endif
