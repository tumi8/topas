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

#include "packetstats.h"


#include <stdexcept>
#include <cstring>

IpfixFile* IpfixFile::ipfixFile = NULL;
Mutex IpfixFile::listLock;
std::list<std::string> IpfixFile::fileNames;


IpfixFile* IpfixFile::writePacket(const char* filename, const byte* data, uint16_t length)
{
        if (!ipfixFile)
                ipfixFile = new IpfixFile();

	/* TODO: Check if file already exists */
	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open()) {
		msg(MSG_ERROR, "Collector: Couldn't open file %s for writing: %s\n", filename, strerror(errno));
                return NULL;
	}
                

	if (!out.write((char*)&length, sizeof(length))) {
		msg(MSG_FATAL, "Collector: Couldn't write packet length to file system: %s\n", strerror(errno));
                return NULL;
	}

	if (!out.write((char*)data, length)) {
		msg(MSG_FATAL, "Collector: Couldn't write packet data to file system: %s\n", strerror(errno));
                return NULL;
	}
                
	out.close();
        listLock.lock();
        fileNames.push_back(filename);
        listLock.unlock();
        
        return ipfixFile;
}

void IpfixFile::proceedOnePacket()
{
        listLock.lock();
	unlink(fileNames.begin()->c_str());
        fileNames.pop_front();
        listLock.unlock();
}


/********************************************************************************/

byte* IpfixShm::startLocation = NULL;
byte* IpfixShm::writePosition = NULL;
byte* IpfixShm::readPosition  = NULL;
bool  IpfixShm::writeBeforeRead = false;
size_t IpfixShm::size = 0;
uint16_t IpfixShm::packetSize = 0;
IpfixShm* IpfixShm::instance = NULL;


IpfixShm::IpfixShm()
{
}

IpfixShm* IpfixShm::writePacket(const byte* data, uint16_t len)
{
        if (!instance) {
                instance = new IpfixShm();
        }
	
        if (len == 0) {
                msg(MSG_ERROR, "IpfixShm: Got empty packet!!!!");
                return NULL;
	}

	if (startLocation == NULL) {
		msg(MSG_ERROR, "IpfixShm: No shared memory storage area allocated!");
                return NULL;
	}

	if (len > size) {
		msg(MSG_ERROR, "IpfixShm: Packet too long for shared memory");
                return NULL;
	}
	
	if ((writePosition + len + sizeof(len)) >= (startLocation + size)) {
		// we only write a complete packet to the shared memory
		// if there is not enough space to do that, we'll start at the
		// the beginning of the buffer
		// We signal this jump to the reader by zeroing the remaining
		// buffer.
		bzero(writePosition, ((startLocation + size) - writePosition));

		if (readPosition >= writePosition) {
			msg(MSG_ERROR, "IpfixShm: Shared memory block too small. Trashing packet!!!!!");
                        return NULL;
		}

		writeBeforeRead = true;
		writePosition = startLocation;
	}

	if (writeBeforeRead && (writePosition + len + sizeof(len)) > readPosition) {
		//msg(MSG_ERROR, "%i %i", (writePosition - startLocation), (readPosition - startLocation));
		msg(MSG_ERROR, "IpfixShm: Shared memory block too small!");
                return NULL;
	}

	//msg(MSG_ERROR, "Writing packet len: %i", len);
	memcpy(writePosition, &len, sizeof(len));
	//msg(MSG_FATAL, "written: %i", *(uint16_t*)writePosition);
	memcpy(writePosition + sizeof(len), data, len);
	//msg(MSG_FATAL, "written: %#06x", ntohs(*(uint16_t*)(writePosition+sizeof(len))));
	writePosition += sizeof(len) + len;

        return instance;
}

uint16_t IpfixShm::readPacket(byte** data) {
	// go to the next packet;
	// packetSize == 0 for the first packet
	readPosition += packetSize;

	if (sizeof(uint16_t) > ((startLocation + size) - readPosition)) {
		writeBeforeRead = false;
		readPosition = startLocation;
	}

	// get the packet length
	packetSize = *(uint16_t*)readPosition;
	readPosition += sizeof(uint16_t);

	if (packetSize == 0) {
		writeBeforeRead = false;
		readPosition = startLocation;
		packetSize = *(uint16_t*)readPosition;
		readPosition += sizeof(uint16_t);
	}
	
	//msg(MSG_FATAL, "reading: %#06x",  ntohs(*(uint16_t*)readPosition));
	*data = readPosition;
	
	
	return packetSize;
}

void IpfixShm::proceedOnePacket()
{
        readPacket(&readPosition);
}

void IpfixShm::setShmPointer(byte* ptr)
{
	startLocation = writePosition = readPosition = ptr;
}

void IpfixShm::setShmSize(size_t s)
{
	size = s;
}
