/**************************************************************************/
/*    Copyright (C) 2006 Romain Michalec                                  */
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
/*    License along with this library; if not, write to the Free Software   */
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,          */
/*    MA  02110-1301, USA                                                 */
/*                                                                        */
/**************************************************************************/

#ifndef _PRINT_STORE_H_
#define _PRINT_STORE_H_

#include <datastore.h>
#include <iostream>

class PrintStore : public DataStore {

 public:

  PrintStore();
  ~PrintStore() {}

  bool recordStart(SourceID);
  void addFieldData(int id, byte* fieldData, int fieldDataLength,
		    EnterpriseNo eid = 0);
  void recordEnd();

  // the following members should be private and all have a 'getter',
  // but for such a simple module, let's not bother about those things...
  uint32_t flowStart;
  uint32_t flowEnd;
  IpAddress sourceAddress;
  IpAddress destinationAddress;
  uint16_t protocol;
  uint16_t sourcePort;
  uint16_t destinationPort;
  uint64_t nb_packets;
  uint64_t nb_bytes;

 private:

  // in the Print module source code (see print-main.cpp), we set AlarmTime
  // to 0 so that the Print::test() method will be called as soon as a record
  // is ready ('real-time monitoring').
  // hence a PrintStore object will contain no more than one record; we use
  // recordNumber as a flag to spot such errors
  unsigned int recordNumber;

  bool recordStarted;

};

#endif
