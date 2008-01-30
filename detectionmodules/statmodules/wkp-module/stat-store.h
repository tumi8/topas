/**************************************************************************/
/*    Copyright (C) 2006-07                                               */
/*    Romain Michalec, Sven Wiebusch                                      */
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
/*    License along with this library; if not, write to the Free Software   */
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,          */
/*    MA  02110-1301, USA                                                 */
/*                                                                        */
/**************************************************************************/

#ifndef _STAT_STORE_H_
#define _STAT_STORE_H_

#include "shared.h"
#include <datastore.h>
#include <concentrator/ipfix.h>
#include <map>
#include <vector>
#include <algorithm> // <vector> does not have a member function find(...)
#include <iostream>
#include <sstream>


// ==================== STORAGE CLASS StatStore ====================

class StatStore : public DataStore {

 private:

  uint64_t packet_nb;               // data coming
  uint64_t byte_nb;                 // from the
                                    // current record
  EndPoint e_source;
  EndPoint e_dest;

  std::map<EndPoint,Info> Data;     // data collected from all records received
                                    // since last call to Stat::test()

  static std::map<EndPoint,Info> PreviousData;
   // data collected from all records received before last call to Stat::test()
   // but not before the call before last call to Stat::test()...
   // that means, PreviousData is short-term memory: it's Data as it was before
   // last call to Stat::test() (and it's static so that it survives the
   // StatStore object destruction that goes with a call to Stat::test())

 public:

  StatStore();
  ~StatStore();
  // it is ~StatStore which updates PreviousData (PreviousData = Data;)

  bool recordStart(SourceID sourceId);
  void recordEnd();
  void addFieldData(int id, byte * fieldData, int fieldDataLength,
		    EnterpriseNo eid = 0);

  std::map<EndPoint,Info> getData() const {return Data;}
  std::map<EndPoint,Info> getPreviousData() const {return PreviousData;}

  bool monitorEndPoint (const EndPoint &);

  friend std::ifstream& operator>>(std::ifstream&, StatStore*);

  static short netmask;
  // will be applied to the ip addresses directly when they are
  // handled in addFieldData(); this is for aggregating ip addresses
  // to subnets

  static bool ipMonitoring;
  static bool portMonitoring;
  static bool protocolMonitoring;
  // these flags will be used for aggregating endpoints in OFFLINE MODE
  // they are initially set to false, i. e. the correspondig member of
  // the endppoint will be set to 0. If the endpoint_key contains the
  // appropriate keys, they are set to true.

  static bool beginMonitoring;
  // Set to "true" by Stat::init() when all the static information about
  // endpoints is ready, else set to "false" by the Stat constructor.
  // It prevents recordStart to give the go-ahead when requested by
  // DetectionBase if something is not ready (cf. Stat::Stat(), where
  // DetectionBase<StatStore> begins to run while Stat::init() is not over).

  static int endPointListMaxSize;
  // This maximal size is defined by the user and set by Stat::init().
  // If forgotten by the user, default max size is provided.

  static bool monitorEveryEndPoint;
  // In case no file is given, then Stat::init() initializes monitorEveryEndPoint
  // to true. Any EndPoint found in a record will be added to the
  // endPointFilter map until a maximal size (to prevent
  // memory exhaustion) is reached.

  static void AddEndPointToFilter (FilterEndPoint fep) {
    endPointFilter.push_back(fep);
  }

  static std::string EndPointFilters();

 private:

  static std::vector<FilterEndPoint> endPointFilter;
  // EndPoints to monitor. They are provided in a file by the user;
  // this file is read by Stat::init(), which then initializes
  // endPointFilter.

  static std::vector<EndPoint> endPointList;
  // Currently monitored EndPoints. Every Endpoint we are interested in
  // is added to this List until EndPointListMaxSize is reached.

  // All these are static because they are the same for every StatStore object.
  // As they will be set by a function, Stat::init(), that doesn't have any
  // StatStore object argument to help call these functions, we absolutely need
  // static and public "setters", wich are programmed hereafter.

};

#endif
