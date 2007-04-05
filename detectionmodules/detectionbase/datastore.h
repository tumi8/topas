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

#ifndef _DATA_STORE_H_
#define _DATA_STORE_H_

#include <concentrator/rcvIpfix.h>
#include <concentrator/ipfix.h>
#include <concentrator/common.h>


#include <map>
#include <vector>
#include <stdexcept>


/**
 * Base class for all storage modules (You don't have to
 * derive your storage module from this class. But your
 * storage class must provide the functions
 * - void addFieldData(int id, byte* fieldData, int len);
 * - bool recordStart(SourceID&);
 * - void recordEnd();
 */
class DataStore 
{
 public:
        /**
         * Constructor 
         */
        DataStore() 
		: valid(false)
	{
	}

        /**
         * Destructor
         */
        ~DataStore() {}

        /**
         * @brief Method used to insert IPFIX data into the module
         * 
         * The method will be called for every field within an Ipfix
         * record.
         * The function will be called quite often.
         * @param id Field id (see concentrator/ipfix.h for a list of available ids
         * @param fieldData transmitted data
         * @param len Size of fieldData
         */
        void addFieldData(int id, byte* fieldData, int len, EnterpriseNo enterprise = 0) {}

        /**
         * Will be called whenever a new record starts.
         * This method can accept or reject a complete record.
         * DetectionBase will call @c recordEnd() before calling
         * recordStart() again.
         *
         * addFieldData() and recordEnd() will not be called, if
         * you reject the record.
         *
         * @return True for accepting the record, false otherwise
         */
        bool recordStart(SourceID) { return true; }

        /**
         * Indicates that all subscribed fields within a record where
         * passed to the module
         * Will only be called after a call to @c recordStart()
         */
        void recordEnd() {}
        

        /**
         * Transforms an IpfixField into an integer (host byte order)
         * @param n Field data to transform
         * @param length Length of field data
         */
       uint64_t fieldToInt(byte* n, unsigned length);

	/**
	 * Checkes wether the buffer is valid or not. A buffer may
	 * be invalid, if it is rejected by recordStart()
	 */
	bool isValid() { return valid; }
	
	/**
	 * Sets validity of the buffer element
	 */
	void setValid(bool v) { valid = v; }

private:
	bool valid;
};



/**
 * Encapsultes an IpAddress (Ipv4)
 */
class IpAddress {
public:
        IpAddress()
       	{
		setAddress(0, 0, 0, 0);
	}

        /**
         * Constructor
         * @param a First byte of address
         * @param b Second byte of address
         * @param c Third byte of address
         * @param d Fourth byte of address
         */
        IpAddress( byte a, byte b, byte c, byte d) 
        {
		setAddress(a, b, c, d);
        }

        /**
         * Constructor
         * @param a 4 byte sized array containing an ip address 
         */
        IpAddress(const byte a[4]) 
        {
		setAddress(a);
        }

        /**
         * Destructor
         */
        ~IpAddress() {};

        /** 
         * Returns field within an ip address. The operator is range checked.
         * @param i Byte to return
         * @return i. byte form ip address
         */
        int operator[](unsigned  i) const
        {
                if (i > 3)
                        throw std::runtime_error("No such field");
                return address[i];
        }

        bool operator==(const IpAddress& ip) const 
        {
                return address[0] == ip.address[0] && address[1] == ip.address[1]
                        && address[2] == ip.address[2] && address[3] == ip.address[3];
        }
        
        bool operator<(const IpAddress& ip) const 
        {
                if (address[0] < ip.address[0])
                        return true;
                if (address[0] == ip.address[0] && address[1] < ip.address[1])
                        return true;
                if (address[0] == ip.address[0] && address[1] == ip.address[1]
		    && address[2] < ip.address[2])
                        return true;
                if (address[0] == ip.address[0] && address[1] == ip.address[1]
		    && address[2] == ip.address[2] && address[3] < ip.address[3])
                        return true;
                return false;
        }
        
        /* TODO: turn this member into an operator */
        std::string toString() const;

	void setAddress(const byte a[4]) 
	{
		setAddress(a[0], a[1], a[2], a[3]);
	}

	void setAddress( byte a, byte b, byte c, byte d) 
	{
                address[0] = a;
                address[1] = b;
                address[2] = c;
                address[3] = d;
	}

	/**
	 * Mask functions
	 * - remanent_mask changes IpAddress object
	 * - mask is only temporary
	 * Warning: netmask is not checked before being applied
	 * 0 <= m1,m2,m3,m4 <= 255 (or 0x00 and 0xFF)
	 */
	IpAddress mask (byte m1, byte m2, byte m3, byte m4) {
	  return IpAddress( address[0] & m1, address[1] & m2, address[2] & m3, address[3] & m4 );
	}
	IpAddress mask (const byte m[4]) {
	  return IpAddress( address[0] & m[0], address[1] & m[1], address[2] & m[2], address[3] & m[3] );
	}
	void remanent_mask (byte m1, byte m2, byte m3, byte m4) {
	  setAddress( address[0] & m1, address[1] & m2, address[2] & m3, address[3] & m4 );
	}
	void remanent_mask (const byte m[4]) {
	  setAddress( address[0] & m[0], address[1] & m[1], address[2] & m[2], address[3] & m[3] );
	}


private:
        int address[4];

        friend std::ostream& operator<<(std::ostream&, const IpAddress&);
};


#endif
