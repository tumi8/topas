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

#ifndef _FILE_POLICY_H_
#define _FILE_POLICY_H_


#include "inputpolicybase.h"
#include "detectcallbacks.h"


#include <commonutils/global.h>
#include <commonutils/sharedobj.h>
#include <commonutils/metering.h>
#include <commonutils/packetstats.h>


#include <sys/types.h>
#include <semaphore.h>
#include <errno.h>


#include <list>
#include <iostream>

/**
 * Uses signals, semaphores and a shared memory block
 * to communicate with the collector.
 * Collector writes its data into files on a filesystem
 * (ramdisk). 
 */ 
class SemShmNotifier : public InputNotificationBase {
public:
        SemShmNotifier();
        ~SemShmNotifier();

        /**
         * Waits for new data
         * @returns always returns 1
         */
        int wait() const;

        /**
         * Informs collector that all data was processed.
         */
        int notify() const;

        /**
         * Returns first filenumber
         * @return First filenumber
         */
        shared::FileCounter getFrom() {
                return nps->from();
        }

        /**
         * Returns last file number
         * @return last file number
         */
        shared::FileCounter getTo() {
                return nps->to();
        }

        /**
         * Get temporary packet direcories, where the IPFIX packets are stored.
         * The directory is provided by the collector
         * @return directory name wherer IPFIX packets are stored
         */
        std::string getPacketDir() {
                return packetDir;
        }

	/**
	 * TODO: REMOVE THIS TESTING WORKAROUND!
	 */
	bool useFiles() { return useFiles_; }
private:
        key_t semKey, shmKey;
        int semId;
        shared::SharedObj* nps;

        std::string packetDir;

	bool useFiles_;
};




/**
 * Extracts whole Ipfix-Packets out of files and imports them directly into
 * a storage class
 */
template <
        class Notifier,
        class Buffer
>
class PacketReader {
public:
        PacketReader()
                : packetProcessor(NULL), data(NULL)
        {
		Metering::setDirectoryName("metering/");
		metering = new Metering("packetreader");
                data = new byte[config_space::MAX_IPFIX_PACKET_LENGTH];

                /* build CallbackInfo */
                CallbackInfo cbi;

                cbi.handle = this;
                
                cbi.templateCallbackFunction = newTemplateArrived<PacketReader, Buffer>;
                cbi.optionsTemplateCallbackFunction = newOptionsTemplateArrived<PacketReader, Buffer>;
                cbi.dataTemplateCallbackFunction = newDataTemplateArrived<PacketReader, Buffer>;
                
                cbi.dataRecordCallbackFunction = newDataRecordArrived<PacketReader, Buffer>;
                cbi.optionsRecordCallbackFunction = newOptionRecordArrived<PacketReader, Buffer>;
                cbi.dataDataRecordCallbackFunction = newDataRecordFixedFieldsArrived<PacketReader, Buffer>;
                
                cbi.templateDestructionCallbackFunction = templateDestroyed<PacketReader, Buffer>;
                cbi.optionsTemplateDestructionCallbackFunction = optionsTemplateDestroyed<PacketReader, Buffer>;
                cbi.dataTemplateDestructionCallbackFunction = dataTemplateDestroyed<PacketReader, Buffer>;
                /*
                  create an packetProcessor and ipfixParser
                  we don't need an receiver because we do the "receiving" work by hand
                */
                IpfixParser* ipfixParser = createIpfixParser();
                addIpfixParserCallbacks(ipfixParser, cbi);
                                
                packetProcessor = createIpfixPacketProcessor();
                setIpfixParser(packetProcessor, ipfixParser);
        }

        ~PacketReader() 
        {
                if (packetProcessor)
                        destroyIpfixPacketProcessor(packetProcessor);
                delete data;
		delete metering;
        }


        void import(Notifier& notifier) {
                static FILE* fd;
                static shared::FileCounter i;

                static int filesize = strlen(notifier.getPacketDir().c_str()) + 30;
                static char* filename = new char[filesize];
		static uint16_t len = 0;

                for ( i = notifier.getFrom(); i != notifier.getTo(); ++i) {
			if (notifier.useFiles()) {
				snprintf(filename, filesize, "%s%i", notifier.getPacketDir().c_str(), (int)i);
				if (NULL == (fd = fopen(filename, "rb"))) {
					std::cerr << "Detection modul: Could not open file"
						  << filename << ": " << strerror(errno) 
						  << std::endl;
				}
				
				read(fileno(fd), &len, sizeof(uint16_t));
				read(fileno(fd), data, len);
				if (isSourceIdInList(*(uint16_t*)(data + 12))) {
					packetProcessor->processPacketCallbackFunction(packetProcessor->ipfixParser, data, len);
				}
				metering->addValue();
				if (EOF == fclose(fd)) {
					std::cerr << "Detection Modul: Could not close "
						  << filename << ": " << strerror(errno)
						  << std::endl;
				}
			} else {
				len = IpfixShm::readPacket(&data);
                                metering->addValue();
				if (isSourceIdInList(*(uint16_t*)(data+12))) {
					packetProcessor->processPacketCallbackFunction(packetProcessor->ipfixParser, data, len);
				}
			}
                }
        }



        void subscribeId(int id) 
        {
                idList.push_back(id);
        }

	void subscribeSourceId(uint16_t id) {
		sourceIdList.push_back(id);
	}

protected:
        std::vector<int> idList;
	std::vector<uint16_t> sourceIdList;
        IpfixPacketProcessor* packetProcessor;
	Mutex recordMutex;
        byte* data;
	Metering* metering;

	virtual Buffer* getBuffer() = 0;


        bool isIdInList(int id) const
        {
		if (idList.empty()) {
			return true;
		}
                /* TODO: think about hashing */
                for (std::vector<int>::const_iterator i = idList.begin(); i != idList.end(); ++i) {
                        if ((*i) == id)
                                return true;
                }
                
                return false;
        }

	bool isSourceIdInList(uint16_t id) const
	{
		if (sourceIdList.empty())
			return true;

		for (std::vector<uint16_t>::const_iterator i = sourceIdList.begin(); i != sourceIdList.end(); ++i) {
			if ((*i) == id) {
				return true;
			}
		}
		return false;
	}

        friend int newTemplateArrived<PacketReader, Buffer>(void* handle,  SourceID sourceID, TemplateInfo* ti);
        friend int newDataRecordArrived<PacketReader, Buffer>(void* handle, SourceID sourceID, TemplateInfo* ti,
							      uint16_t length, FieldData* data);
        friend int templateDestroyed<PacketReader, Buffer>(void* handle, SourceID sourceID, TemplateInfo* ti);
        friend int newOptionsTemplateArrived<PacketReader, Buffer>(void* handle, SourceID sourceID, OptionsTemplateInfo* optionsTemplateInfo);
        friend int newOptionRecordArrived<PacketReader, Buffer>(void* handle, SourceID sourceID, OptionsTemplateInfo* oti,
								uint16_t length, FieldData* data);
        friend int optionsTemplateDestroyed<PacketReader, Buffer>(void* handle, SourceID sourceID, OptionsTemplateInfo* optionsTemplateInfo);
        friend int newDataTemplateArrived<PacketReader, Buffer>(void* handle, SourceID sourceID, DataTemplateInfo* dataTemplateInfo);
        friend int newDataRecordFixedFieldsArrived<PacketReader, Buffer>(void* handle, SourceID sourceID,
									 DataTemplateInfo* ti, uint16_t length,
									 FieldData* data);
        friend int dataTemplateDestroyed<PacketReader, Buffer>(void* handle, SourceID sourceID, DataTemplateInfo* dataTemplateInfo);
};



/** 
 * Extracts IPFIX packets from files and imports them direcly into a storage
 * class. All data is buffered into one storage class till the data is fetched
 * using @c getStorage().
 */ 
template <
	class Notifier,
	class Storage
>
class BufferedFilesInputPolicy : public InputPolicyBase<Notifier, Storage>, public PacketReader<Notifier, Storage> {
public:
	BufferedFilesInputPolicy() {
		buffer = new Storage();
	}

	~BufferedFilesInputPolicy() {
		if (buffer) 
			delete buffer;
	}

	void importToStorage() {
		packetLock.lock();
		import(this->getNotifier());
		packetLock.unlock();
	}

	/**
	 * Returns a storage object. This object contains all data buffered since last call to @c getStorage().
	 * The method returns an empty object if no data was buffered.
	 * @return buffered IFPIX data.
	 */
        Storage* getStorage()
        {
                packetLock.lock();
                Storage* ret = buffer;
                buffer = new Storage();
                packetLock.unlock();
                return ret;
        }
		
private:
	Storage* buffer;
	Mutex packetLock;

	Storage* getBuffer() { return buffer; }
};



/** 
 * Extracts IPFIX packets from files and imports them into a storage class. 
 * Each IPFIX record is seperately stored within an instance of the Storage class.
 */ 
template <
	class Notifier,
	class Storage
>
class UnbufferedFilesInputPolicy : public InputPolicyBase<Notifier, Storage>, public PacketReader<Notifier, Storage> {
public:
	UnbufferedFilesInputPolicy() : maxBuffers(256), bufferErrors(0) {
		packetLock.lock();
	}

	~UnbufferedFilesInputPolicy() {
	}

	void importToStorage() {
		import(this->getNotifier());
	}

	/**
	 * Returns storage if there are new records available. Blocks if no records available.
	 */
        Storage* getStorage()
        {
		packetLock.lock();
		PacketReader<Notifier, Storage>::recordMutex.lock();
		Storage* ret = *buffers.begin();
		buffers.pop_front();
		PacketReader<Notifier, Storage>::recordMutex.unlock();
		return ret;
        }

private:
	Storage* getBuffer() {
		if(buffers.size() >= maxBuffers) // Buffer is full
		{
			bufferErrors++;
			msg(MSG_ERROR, "DetectionBase: getBuffer() returns NULL, record will be dropped! %lu", bufferErrors);
			return NULL;
		}
		Storage* ret = new Storage();
		buffers.push_back(ret);
		packetLock.unlock();
		return ret;
	}

	std::list<Storage*> buffers;
	unsigned maxBuffers;	
	unsigned bufferErrors;
	Mutex packetLock; // locked as long as buffers is empty
};

#endif
