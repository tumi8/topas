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

#include "recorder.h"


#include <commonutils/packetstats.h>


#include <sys/time.h>


#include <stdexcept>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <algorithm>


FileRecorder::FileRecorder(const std::string& s, bool rec)
        : RecorderBase(rec), number(0)
{
        startTime = usecs();
        storagePath = s;
	if (recording) {
		indexFile.open((storagePath + "index").c_str(), std::ios::out);
	}  else {
		indexFile.open((storagePath + "index").c_str(), std::ios::in);
	}
        if (!indexFile.is_open()) {
                throw std::runtime_error(std::string("FileRecorder: Could not open index file: ") + 
					 storagePath + "index");
	}
	fileName = new char[storagePath.length() + 30];
	fileNameSize = storagePath.length() + 30;
}


FileRecorder::~FileRecorder()
{
	if (!recording) {
		msg(MSG_INFO, "Calculating drift times in replaying process ... ");
		msg(MSG_DEBUG, "Minimum drift time: %i ms", *std::min_element(times.begin(), times.end()));
		msg(MSG_DEBUG, "Maximum drift time: %i ms", *std::max_element(times.begin(), times.end()));
                // write the in a file
		std::ofstream outfile("filerecorder.diffs");
		unsigned res = 0;
		for (unsigned i = 0; i != times.size(); ++i) {
			res += times[i];
			outfile << times[i] << std::endl;
		}
		msg(MSG_INFO, "Arithmetic mean drift time was %i ms.", res/times.size());

		res = 0;
		msg(MSG_DEBUG, "Geometric mean drift time was %i ms.", res);
	}
	delete fileName; fileName = 0;
}


unsigned int FileRecorder::usecs()
{
	static struct timeval tv;
	gettimeofday(&tv, 0);
	return (tv.tv_sec * 1000000) + (tv.tv_usec);
}

void FileRecorder::record(const byte* data, uint16_t len)
{
	if (!recording) {
		return;
	}
	static long aktTime;
	aktTime = usecs();
	
	snprintf(fileName, fileNameSize, "%s%lu", storagePath.c_str(), number);
	IpfixFile::writePacket(fileName, data, len);
	indexFile << aktTime - startTime << "\t" << number << std::endl;
	number++;
}

void FileRecorder::play()
{
	std::string line;
	FILE* fd;
	byte* data = new byte[config_space::MAX_IPFIX_PACKET_LENGTH];
	unsigned int time;
	unsigned long fileNumber;
	std::ofstream originalValues("filerecorder.orig");
	std::ofstream replayValues("filerecorder.replay");
	while (std::getline(indexFile, line) && !do_abort) {
		sscanf(line.c_str(), "%u %lu", &time, &fileNumber);

		snprintf(fileName, fileNameSize, "%s%lu", storagePath.c_str(), fileNumber);


		if (NULL == (fd = fopen(fileName, "rb"))) {
			delete data; data = 0;
			throw std::runtime_error(std::string("FileRecorder: Could not open file: ") + fileName);
		}

		uint16_t len;
		read(fileno(fd), &len, sizeof(uint16_t));
		read(fileno(fd), data, len);
		if (EOF == fclose(fd)) {
			delete data; data = 0;
			throw std::runtime_error(std::string("FileRecorder: Could not open file: ") + fileName);
		}

		static unsigned int t;
		if ((usecs() - startTime) < time) {
			if (-1 == usleep(time - t)) {
				msg(MSG_ERROR, "Error waiting for recording time");
			}
		}
		t = usecs() - startTime;
		//std::cout << ((int)t - (int)time) / 1000 << std::endl;
		times.push_back(((int)t - (int)time) / 1000);
		originalValues << time / 1000 << std::endl;
		replayValues   << t    / 1000 << std::endl;
		
		if (packetCallback) {
			packetCallback(NULL, data, len);
		}

	}
	delete data; data = 0;
}
