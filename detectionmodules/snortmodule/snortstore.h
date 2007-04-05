/**************************************************************************/
/*   Copyright (C) 2006-2007 Nico Weber                                   */
/*                                                                        */
/*   This library is free software; you can redistribute it and/or        */
/*   modify it under the terms of the GNU Lesser General Public           */
/*   License as published by the Free Software Foundation; either         */
/*   version 2.1 of the License, or (at your option) any later version.   */
/*                                                                        */
/*   This library is distributed in the hope that it will be useful,      */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*   Lesser General Public License for more details.                      */
/*                                                                        */
/*   You should have received a copy of the GNU Lesser General Public     */
/*   License along with this library; if not, write to the Free Software  */
/*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,           */
/*   MA  02110-1301, USA                                                  */
/**************************************************************************/

#ifndef _SNORTSTORE_H_
#define _SNORTSTORE_H_

#include <datastore.h>
#include "pcappacket.h"
#include <time.h>
#include <vector>
/**\brief Snortmodule datastorage class
 *
 * Used to assemble and store one packet which is later processed by the writer.
 */

class SnortStore : public DataStore {
public:
        SnortStore();
        ~SnortStore();

        bool recordStart(SourceID);
        void recordEnd();

        void addFieldData(int id, byte* fieldData, int fieldDataLength, EnterpriseNo eid = 0); ///< Used by the collector to store data

	PcapPacket* get_record(); ///< Used to get the record
	bool is_valid;	
private:
	PcapPacket *packet; 

};

#endif
