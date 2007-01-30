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

#include "detectmodexporter.h"
#include "manager.h"
#include "detectmod.h"
#include "modulecontainer.h"


#include <concentrator/rcvIpfix.h>
#include <concentrator/msg.h>
#include <commonutils/global.h>
#include <commonutils/packetstats.h>


#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>


#include <string>
#include <sstream>


DetectModExporter::DetectModExporter()
	: nps(NULL), exchangeStyle(USE_FILES)
{
        nps = new shared::SharedObj();
        shmKey = nps->getShmKey();
}


DetectModExporter::~DetectModExporter()
{
        delete nps; nps = 0;
}

int DetectModExporter::exportToSink(IpfixParser*, const byte* data, uint16_t len) {

        uint32_t sourceId = ntohl(*(uint32_t*)(data+12)); // see Ipfix-Protocol

	if (exchangeStyle == USE_FILES) {
                static shared::FileCounter counter;
                static IpfixFile* ipfixFile = NULL;
                static int filesize = strlen(packetDir.c_str()) + 30;
                static char* filename = new char[filesize];

                snprintf(filename, filesize, "%s%i", packetDir.c_str(), (int)counter);
                ipfixFile = IpfixFile::writePacket(filename, data, len);
                if (ipfixFile) {
	                ipfixPacketStore.pushIpfixPacket(sourceId, ipfixFile);
                } else {
                        return -1;
                }
                counter++;
	} else {
                static IpfixShm* ipfixShm = NULL;
                ipfixShm = IpfixShm::writePacket(data, len);
                if (ipfixShm) {
        		ipfixPacketStore.pushIpfixPacket(sourceId, ipfixShm);
                } else {
                        return -1;
                }
        }
        return 0;
}

void DetectModExporter::installNotification(DetectMod& detectMod) const {
        detectMod.setShmKey(shmKey);
}


void DetectModExporter::notify(DetectMod* module)
{
        static struct sembuf semaphore;
        /* write packet informations into the shared memory */
        /* TODO: select proper queue and write queue number into  */
        nps->setFrom(ipfixPacketStore.getPacketStats(0).oldest);
        nps->setTo(ipfixPacketStore.getPacketStats(0).newest);

	/* set semaphore and call the detection modules */
	semaphore.sem_num = 0;
	semaphore.sem_op = 2;
	semaphore.sem_flg = 0;
	if (-1 == semop(module->getSemId(), &semaphore, 1)) {
		msg(MSG_ERROR, "Manager: Error setting semaphore: %s\n"
                    "This could be responsible for killing %s in the future",
		    strerror(errno), module->getFileName().c_str());
	}
}
 
int DetectModExporter::wait(DetectMod* module)
{
        static struct sembuf semaphore;
        static struct timespec t;
        static bool wait;
        /*
          loop through all detection modules and wait for them to decrement their semaphores.
          if the detection modules aren't able to parse the packets within killTime seconds
          the manager will kill the module!!!
        */
	semaphore.sem_num = 0;
	semaphore.sem_op = 0;
	semaphore.sem_flg = 0;
        t.tv_sec = 0;
        t.tv_nsec = 10000;
        wait = true;
        while (wait) { 
        	if (-1 == semtimedop(module->getSemId(), &semaphore, 1, &t)) {
                        if (errno == EAGAIN && module->getState() == DetectMod::Running) {
                               continue; 
                        }

        		msg(MSG_ERROR, "Manager: Error setting semaphore: %s\n"
                            "This could be responsible for killing %s in the future",
                	    strerror(errno), module->getFileName().c_str());
                        return -1;
                } else
                        wait = false;
                
	}

        return 0;
}

void DetectModExporter::clearSink()
{
	ipfixPacketStore.popIpfixPacket(0, nps->to() - nps->from());
}


void DetectModExporter::sendInitData(const DetectMod& detectMod, const std::string& additionalData)
{
        /* send semaphore key, shared memory key and packetdir to the detection module*/
        /* TODO: implement timeout (if we don't, we will hang if the detection module doesn't read from its pipe) */
        std::string tmp;
        std::stringstream ss;
        ss << detectMod.getSemKey() << " " << detectMod.getShmKey() << " ";
	if (exchangeStyle == USE_FILES) {
		ss << "USE_FILES ";
	} else {
		ss << "USE_SHM ";
	}
        tmp =  ss.str();
        write(detectMod.getPipeFd(), tmp.c_str(), tmp.size());
        /* the modules expect to read a packetDir. If we use shared memory to exchange
           the IPFIX packets, we don't have a packetDir. But the modules expects one, so
           we are sending a dummy string
        */
        tmp = (packetDir.empty()?"dummy_string":packetDir) + "\n";
        write(detectMod.getPipeFd(), tmp.c_str(), tmp.size());
        if (!additionalData.empty()) {
                tmp = additionalData + "\n";
                write(detectMod.getPipeFd(), tmp.c_str(), tmp.size());
        }
}

void DetectModExporter::setPacketDir(const std::string& dir)
{
        packetDir = dir;
}

std::string DetectModExporter::getPacketDir()
{
        return packetDir;
}

void DetectModExporter::setExportingStyle(ExchangeStyle e)
{
	exchangeStyle = e;
}

void DetectModExporter::setSharedMemorySize(size_t size)
{
	int keyNo = 1;
	bool searchFreeKey = true;
	int id = -1;
	do {
		if (-1 == (id = shmget(keyNo, size, S_IRWXU | IPC_CREAT | IPC_EXCL))) {
			if (errno == EEXIST) {
				// try another key ...
				++keyNo;
				continue;
			} else {
				throw std::runtime_error(std::string("DetectModExporter: Cannot allocate shared memory block: ") + strerror(errno));
			}
		} else 
			searchFreeKey = 0;
	} while (searchFreeKey);

	nps->setStorageKey(keyNo);
	nps->setStorageSize(size);

	void* shmPtr = shmat(id, NULL, 0);
	if ((void*)-1 == shmPtr) {
		throw std::runtime_error(std::string("DetectModExporter: Could not attach shared memory storage area: ") + strerror(errno));
	}

	IpfixShm::setShmPointer((byte*)shmPtr);
	IpfixShm::setShmSize(size);
}
