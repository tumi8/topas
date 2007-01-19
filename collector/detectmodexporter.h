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

#ifndef _DETECTMODEXPORTER_H_
#define _DETECTMODEXPORTER_H_


#include <commonutils/packetstats.h>
#include <concentrator/rcvIpfix.h>


class ModuleContainer;
class DetectMod;


/**
 * Class responsible for exporting the collected data to the detection modules.
 * The class itself is not threadsafe!
 */
class DetectModExporter {
public:
	/**
	 * Defines exporting style.
	 *
	 * There are two existing exchange styles. The first (USE_FILES) will
	 * exchange IPFIX Packets via a file system (most likely on a RAM-Disk).
	 * The second exchange method uses a shared memory block to copy the data
	 * to the modules.
	 */
	typedef enum {
		USE_FILES,
		USE_SHARED_MEMORY
	} ExchangeStyle;


	/**
	 * Constructor creates and initialises the shared memory block which is
	 * necessaryto communicate with all the detection modules.
	 */
        DetectModExporter();

	/**
	 * Deletes the allocated shared memory block. All detection modules should
	 * be terminated before deleting this object (otherwise all operations on
	 * the shared memory will fail.
	 */
        ~DetectModExporter();

	/**
	 * Inserts the IPFIX data into the data sink. The modules will not be notified
	 * about the new data arriving at the sink. Use @c notifyAll() to perform
	 * the notification process.
	 * @param ipfixParser Not used.
	 * @param data IPFIX data (likely one IPFIX packet)
	 * @param len Length of IPFIX data.
	 */
        int exportToSink(IpfixParser* /*ipfixParser*/, const byte* data, uint16_t len);

	/**
	 * Clears all processed data from the data sink.
	 */
	void clearSink();

        /**
	 * Informes one detection module about new incoming data. The method does not
	 * guaranty that the module got the notification.
	 * @param module The module that should be informed.
	 */
	void notify(DetectMod* module);

        /**
	 * Blocks until the module processed its data.
	 */
	void wait(DetectMod* module);

	/**
	 * Performes necessary work before a module can be notfied.
	 */
        void installNotification(DetectMod& detectMod) const;

	/**
	 * Sends the information about semaphores and the shared memory
	 * object to all modules.
	 */
        void sendInitData(const DetectMod&);

        /* all files in this directory will be deleted */
        void setPacketDir(const std::string&);

        std::string getPacketDir();

	/**
	 * Set memory size. This function already allocates the memory
	 */
	void setSharedMemorySize(size_t size);

	/**
	 * sets exporting style. This can be either DetectModExporter::USE_FILES
	 * or DetectModExporter::USE_SHARED_MEMORY
	 * @param e the exporting style
	 */
	void setExportingStyle(ExchangeStyle e);

private:
        IpfixPacketStore ipfixPacketStore;

        key_t shmKey;
        int shmId;
        
        shared::SharedObj* nps;

        std::string packetDir;

	ExchangeStyle exchangeStyle;
};

#endif
