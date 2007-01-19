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

#ifndef _RECORDER_H_
#define _RECORDER_H_


#include <concentrator/rcvIpfix.h>


#include <string>
#include <fstream>
#include <vector>


/**
 * Base class for all recording/playing modules. This is a pure interface
 * class, which provides methods for recording and replaying IPFIX packets.
 */
class RecorderBase {
public:
	enum {
		PrepareReplaying = false,
		PrepareRecording = true
	};
		
	/**
	 * Constructor...
	 * @param rec Specifies if recorder is used for recording or for reading files.
	 */
	RecorderBase(bool rec = true) : packetCallback(0), recording(rec), do_abort(false) {}

	/**
	 * Virtual destructor...
	 */
        virtual ~RecorderBase() {};

	/**
	 * Interface method for recording IPFIX packets. Every incoming IPFIX packet
	 * will be passed to this function before it is passed to the detection modules.
	 * @param data Ipfix packet data
	 * @param len Length of data.
	 */
        virtual void record(const byte* data, uint16_t len) = 0;

	/**
	 * Interface method for replaying recorded IPFIX packets. This method is intended
	 * to run insteed of the network collecting part of the concentrator.
	 */
	virtual void play() = 0;


	void abort() {do_abort = true;}

	/**
	 * Sets the callback function which is called whenever a packet has to be sent to
	 * the collector when replaying IPFIX traffic.
	 */
	void setPacketCallback(ProcessPacketCallbackFunction* pp) {
		packetCallback = pp;
	}

protected:
	ProcessPacketCallbackFunction* packetCallback;
	bool recording;
	volatile bool do_abort;
};


/**
 * Dummy recording module. This module should be created when, no
 * recording is wanted. 
 */
class RecorderOff : public RecorderBase {
public:
	RecorderOff(bool recording = true) {}
	/**
	 * empty virtual destructor
	 */
        virtual ~RecorderOff() {};

	/**
	 * Dummy recording function. It does not perform any recording at all.
         * @param data Ipfix packet data
         * @param len Length of data.       
	 */
        virtual void record(const byte* data, uint16_t len) {}

	/**
	 * Dummy play function. Will return immediately.
	 */
	virtual void play() {}
};


/**
 * Recording class which stores all IPFIX packets into seperat files into one directory.
 * The packets are numbered in an strict ascending order and the files are named according
 * to their numbers.
 * The class will put an index file into the recording directory and records the time at which
 * the packet was recorded (time is in microseconds from program start)
 * When replaying the IPFIX traffic, packets will bepassed to the callback function according
 * to their recording time.
 */
class FileRecorder : public RecorderBase {
public:
	/**
	 * Constructor
	 * Creates index file.
	 * @param s Path to the directory the IPFIX packets are stored.
	 * @param recording Specifies if recorder is used for recording or for reading files.
	 */
        FileRecorder(const std::string& s, bool rec);

	/**
	 * Destructor
	 * Closes index file
	 */
        virtual ~FileRecorder();

	/**
	 * Stores IPFIX packets into the directory specified in the constructor. The incoming packets
	 * are numbered in a strict ascending order and the files are named according to this number.
         * @param data Ipfix packet data
         * @param len Length of data.	 
	 */
        virtual void record(const byte* data, uint16_t len);

	/**
	 * Replays the recorded IPFIX packets. They are passed to the collector depending on the time
	 * recorded in the index file.
	 */ 
	virtual void play();

private:
        unsigned int usecs();

        unsigned int startTime;
        std::string storagePath;
        unsigned long number;
        std::fstream indexFile;
	size_t fileNameSize;
	char* fileName;

	std::vector<int> times;
};

#endif
