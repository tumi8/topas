/**************************************************************************/
/*    Copyright (C) 2007 Gerhard Muenz                                    */
/*    University of Tuebingen, Germany                                    */
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

#ifndef _MSGSTREAM_H_
#define _MSGSTREAM_H_

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

/**
 * Message Stream class as a C++ alternative to the msg(...) function of msg.h/c
 *
 * This class allows printing error, info and debug messages to stdout.
 * Currently, five messaging levels are defined in enum type MsgLevel:
 *   FATAL, ERROR, WARN, INFO, DEBUG
 * The default level is WARN, which means that FATAL, ERROR and WARN level
 * messages are printed. You can change the messaging level with
 * setLevel(MsgLevel).
 * The method setName(const std::string&) sets a name (typically the module or
 * program name) which is included in every output.
 * There are two ways to print a message:
 * (i)  Use the method print(MsgLevel level, const std::string& msg) to print
 *      a string.
 * (ii) Use the streaming operator << to generate more flexible output.
 *      The first input to the stream should be the level of the message, i.e.
 *      MsgStream::FATAL. To terminate the line, use MsgStream::endl.
 *
 * Usage example:
 *   MsgStream ms;
 *   ms.setLevel(MsgStream::INFO);
 *   ms.setName("my module");
 *   ms.print(MsgStream::INFO, "Just a short note.");
 *   ms << MsgStream::FATAL << "I have to die!" << MsgStream::endl;
 *
 * Logging to a file:
 * Use setLogLevel(MsgLevel) and openLogfile(filename) to start logging the
 * messages at a given level into a logfile.
 *
 * Supressing intro string:
 * Use rawPrint() or start stream with MsgStream::raw.
 */

class MsgStream {
    public:
	typedef enum {
	    NONE = 0, FATAL=1, ERROR=2, WARN=3, INFO=4, DEBUG=5
	} MsgLevel;

	typedef enum {endl, raw} MsgControl;

	/**
	 * Creates a new message stream.
	 */
	MsgStream() : name("unknown"), outputLevel(WARN), printThis(false), 
		      logLevel(NONE), logThis(false), noIntro(false) {}
	MsgStream(MsgLevel l, std::string s) : name(s), outputLevel(l), printThis(false),
					       logLevel(NONE), logThis(false), noIntro(false) {}

	/**
	 * Destroyes the message stream
	 */
	~MsgStream() {logfile.close();}

	/**
	 * Sets the name of the module issuing the messages.
	 * @name new name
	 */
	void setName(const std::string& newname);

	/**
	 * Sets the messaging level.
	 * @level new messaging level
	 */
	void setLevel(MsgLevel level);

	/**
	 * Get the messaging level.
	 * @returns messaging level 
	 */
	MsgLevel getLevel() {return outputLevel;}

	/**
	 * Opens the logfile and starts logging.
	 * @name new name
	 * @returns true on success
	 */
	bool openLogfile(const std::string& filename);

	/**
	 * Closes the logfile and stops logging.
	 */
	void closeLogfile();

	/**
	 * Get logfile ofstream.
	 * @returns logfile ofstream
	 */
	std::ofstream& getLogfile() {return logfile;}

	/**
	 * Sets the logging level for the Logfile.
	 * @level new messaging level
	 */
	void setLogLevel(MsgLevel level);

	/**
	 * Get the logging level.
	 * @returns logging level 
	 */
	MsgLevel getLogLevel() {return logLevel;}

	/**
	 * Print a message at given messaging level.
	 * @level messaging level
	 * @msg message to print
	 */
	void print(MsgLevel level, const std::string& msg);

	/**
	 * Print a message at given messaging level without intro.
	 * @level messaging level
	 * @msg message to print
	 */
	void rawPrint(MsgLevel level, const std::string& msg);

	friend MsgStream& operator<<(MsgStream&, MsgStream::MsgLevel);
	friend MsgStream& operator<<(MsgStream&, MsgStream::MsgControl);
	friend MsgStream& operator<<(MsgStream&, const std::string&);
	friend MsgStream& operator<<(MsgStream&, int16_t);
	friend MsgStream& operator<<(MsgStream&, uint16_t);
	friend MsgStream& operator<<(MsgStream&, int32_t);
	friend MsgStream& operator<<(MsgStream&, uint32_t);
	friend MsgStream& operator<<(MsgStream&, int64_t);
	friend MsgStream& operator<<(MsgStream&, uint64_t);
	friend MsgStream& operator<<(MsgStream&, float);
	friend MsgStream& operator<<(MsgStream&, double);

    private:
	void printIntro(MsgLevel level);
	void logIntro(MsgLevel level);

	std::string name;

	MsgLevel outputLevel;
	bool printThis;

	std::ofstream logfile;
	MsgLevel logLevel;
	bool logThis;

	bool noIntro;
};

inline MsgStream& operator<<(MsgStream& ms, MsgStream::MsgLevel input)
{
    if(input <= ms.outputLevel)
    {
	if(!ms.noIntro)
	    ms.printIntro(input);
	ms.printThis = true;
    }
    if(ms.logfile.is_open() && (input <= ms.logLevel))
    {
	if(!ms.noIntro)
	    ms.logIntro(input);
	ms.logThis = true;
    }
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, MsgStream::MsgControl input)
{
    if((ms.printThis) && (input == MsgStream::endl)) {
	std::cout << std::endl;
	ms.printThis = false;
    }
    if((ms.logThis) && (input == MsgStream::endl)) {
	ms.logfile << std::endl;
	ms.logThis = false;
    }
    if(input == MsgStream::raw)
	ms.noIntro = true;
    else
	ms.noIntro = false;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, const std::string& input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, int16_t input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, uint16_t input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, int32_t input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, uint32_t input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, int64_t input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, uint64_t input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, float input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, double input)
{
    if(ms.printThis)
	std::cout << input;
    if(ms.logThis)
	ms.logfile << input;
    return ms;
}

#endif
