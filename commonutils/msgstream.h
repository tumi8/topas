/**************************************************************************/
/*    Copyright (C) 2007 Gerhard Muenz                                    */
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
 */

class MsgStream {
public:
	typedef enum {
	    FATAL=1, ERROR=2, WARN=3, INFO=4, DEBUG=5
	} MsgLevel;
	
	typedef enum {endl} MsgControl;
	
	/**
	 * Creates a new message stream.
	 */
	MsgStream() : outputLevel(WARN), name("unknown"), printThis(false) {}
	MsgStream(MsgLevel l, std::string s) : outputLevel(l), name(s), printThis(false) {}

	/**
	 * Destroyes the message stream
	 */
	~MsgStream() {}

	/**
	 * Sets the messaging level.
	 * @level new messaging level
	 */
	void setLevel(MsgLevel level);

	/**
	 * Sets the name of the module issuing the messages.
	 * @name new name
	 */
	void setName(const std::string& newname);

	/**
	 * Print a message at given messaging level.
	 * @level messaging level
	 * @msg message to print
	 */
	void print(MsgLevel level, const std::string& msg);


private:
	void printIntro(MsgLevel level);
	    
	MsgLevel outputLevel;
	std::string name;
	bool printThis;

	friend MsgStream& operator<<(MsgStream&, MsgStream::MsgLevel);
	friend MsgStream& operator<<(MsgStream&, MsgStream::MsgControl);
	friend MsgStream& operator<<(MsgStream&, const std::string&);
	friend MsgStream& operator<<(MsgStream&, int16_t);
	friend MsgStream& operator<<(MsgStream&, uint16_t);
	friend MsgStream& operator<<(MsgStream&, int32_t);
	friend MsgStream& operator<<(MsgStream&, uint32_t);
	friend MsgStream& operator<<(MsgStream&, int64_t);
	friend MsgStream& operator<<(MsgStream&, uint64_t);
};

inline MsgStream& operator<<(MsgStream& ms, MsgStream::MsgLevel input)
{
    if(input <= ms.outputLevel)
    {
	ms.printIntro(input);
	ms.printThis = true;
    }
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, MsgStream::MsgControl input)
{
    if((ms.printThis) && (input == MsgStream::endl))
	std::cout << std::endl;
    ms.printThis = false;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, const std::string& input)
{
    if(ms.printThis)
	std::cout << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, int16_t input)
{
    if(ms.printThis)
	std::cout << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, uint16_t input)
{
    if(ms.printThis)
	std::cout << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, int32_t input)
{
    if(ms.printThis)
	std::cout << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, uint32_t input)
{
    if(ms.printThis)
	std::cout << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, int64_t input)
{
    if(ms.printThis)
	std::cout << input;
    return ms;
}

inline MsgStream& operator<<(MsgStream& ms, uint64_t input)
{
    if(ms.printThis)
	std::cout << input;
    return ms;
}

#endif 
