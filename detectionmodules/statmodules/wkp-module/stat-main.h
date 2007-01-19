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

#ifndef _STAT_MAIN_H_
#define _STAT_MAIN_H_

#include "stat-store.h"
#include <datastore.h>
#include <detectionbase.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <algorithm> // sort(...), unique(...)


// ==================== CLASS DirectedIpAddress ====================

// we use this class as key field in std::map< key, value >

#define OUT -1
#define IN 1
#define DEFAULT 0

class DirectedIpAddress : public IpAddress {

private:

  int Direction; // = OUT or IN, more rarely DEFAULT

public:

  DirectedIpAddress (byte, byte, byte, byte, int dir);
  DirectedIpAddress (const byte tab[4], int dir);
  DirectedIpAddress (const IpAddress &, int dir);
  ~DirectedIpAddress () {}

  void setDirectedIpAddress (byte, byte, byte, byte, int dir);
  void setDirectedIpAddress (const byte tab[4], int dir);
  void setDirectedIpAddress (const IpAddress &, int dir);

  int getDirection () const {return Direction;}

  // mask functions:
  // - remanent_mask changes DirectedIpAddress object
  // - mask is only temporary
  // warning: netmask is not checked before being applied
  // use only 0 <= m1,m2,m3,m4 <= 255 (or 0x00 and 0xFF)
  DirectedIpAddress mask (byte, byte, byte, byte);
  DirectedIpAddress mask (const byte m[4]);
  void remanent_mask (byte, byte, byte, byte);
  void remanent_mask (const byte m[4]);

  // we need an order relation to use DirectedIpAddress
  // as key field in std::map< key, value >, so:
  bool operator == (const DirectedIpAddress &) const;
  bool operator < (const DirectedIpAddress &) const;

};

std::ostream & operator << (std::ostream &, const DirectedIpAddress & DirIP);


// ======================== STRUCT Samples ========================

// we use this structure as value field in std::map< key, value >

struct Samples {
  std::list<int64_t> Old;
  std::list<int64_t> New;
};


// ========================== CLASS Stat ==========================

#define DEFAULT_warning_verbosity 0
#define DEFAULT_output_verbosity 3
#define DEFAULT_iplist_maxsize 421
#define DEFAULT_sample_old_size 111
#define DEFAULT_sample_new_size 11
#define DEFAULT_stat_test_frequency 1

// main class: does tests, stores samples,
// reads and stores test parameters...

class Stat : public DetectionBase<StatStore> {


 public:


  Stat(const std::string & configfile);
  ~Stat() {}

  /* Test function. This function will be called in periodic intervals.
   *   Set interval size with @c setAlarmTime().
   * @param store : Pointer to a storage object of class StatStore containing
   *   all data collected since last call to test. This pointer points to
   *   your memory. That means that you may store the pointer.
   *   IMPORTANT: _YOU_ have to free the object's memory when you don't need
   *   it any longer (use delete-operator to do that). */
  void test(StatStore * store);


 private:


  // this function is called by the Stat constructor, its job is to extract
  // user's preferences and test parameters from the XML config file:
  void init(const std::string & configfile);

  // as the init function is really huge, we divide it into tasks:
  // those related to the user's preferences regarding the module...
  void init_output_file(ConfObj *);
  void init_alarm_time(ConfObj *);
  void init_warning_verbosity(ConfObj *);
  void init_output_verbosity(ConfObj *);
  void init_monitored_values(ConfObj *);
  void init_noise_thresholds(ConfObj *);
  void init_protocols(ConfObj *);
  void init_netmask(ConfObj *);
  void init_ports(ConfObj *);
  void init_ip_addresses(ConfObj *);
  void init_stat_test_freq(ConfObj *);
  void init_report_only_first_attack(ConfObj *);
  void init_pause_update_when_attack(ConfObj *);

  // ... and those related to the statistical tests parameters
  void init_which_test(ConfObj *);
  void init_sample_sizes(ConfObj *);
  void init_two_sided(ConfObj *);
  void init_significance_level(ConfObj *);


  // the following functions are called by the test()-function:
  std::map<DirectedIpAddress,int64_t> extract_data (StatStore *);
  void update(std::list<int64_t> &, std::list<int64_t> &, int64_t);
  void stat_test(std::list<int64_t> &, std::list<int64_t> &);

  // the following functions are called by the extract_data()-function:
  void extract_data_packets (
    StatStore *, std::map<DirectedIpAddress,int64_t> &);
  void extract_data_octets (
    StatStore *, std::map<DirectedIpAddress,int64_t> &);
  void extract_data_octets_per_packets (
    StatStore *, std::map<DirectedIpAddress,int64_t> &);
  void extract_data_packets_out_minus_packets_in (
    StatStore *, std::map<DirectedIpAddress,int64_t> &);
  void extract_data_octets_out_minus_octets_in (
    StatStore *, std::map<DirectedIpAddress,int64_t> &);
  void extract_packets_t_minus_packets_t_1 (
    StatStore *, std::map<DirectedIpAddress,int64_t> &);
  void extract_octets_t_minus_octets_t_1 (
    StatStore *, std::map<DirectedIpAddress,int64_t> &);

  // and the following functions are called by the stat_test()-function:
  void stat_test_wmw(std::list<int64_t> &, std::list<int64_t> &);
  void stat_test_ks (std::list<int64_t> &, std::list<int64_t> &);
  void stat_test_pcs(std::list<int64_t> &, std::list<int64_t> &);


  // here is the sample container:
  std::map<DirectedIpAddress, Samples> Records;


  // user's preferences (defined in the XML config file):
  std::ofstream outfile;
  int warning_verbosity;
  int output_verbosity;
  std::string monitored_value;
  int noise_threshold_packets;
  int noise_threshold_bytes;
  std::string ipfile;
  int iplist_maxsize;
  int stat_test_frequency;
  bool report_only_first_attack;
  bool pause_update_when_attack;


  // test parameters (defined in the XML config file):
  bool enable_wmw_test;
  bool enable_ks_test;
  bool enable_pcs_test;
  int sample_old_size;
  int sample_new_size;
  bool two_sided;
  double significance_level;


  // the following counter enables us to call a statistical test every
  // X=stat_test_frequency times that the test() method is called,
  // rather than everytime
  int test_counter;

  // port-monitoring flag: set to false at the beginning of
  // init_protocols() and then to true if TCP or UDP protocols are
  // monitored; enables init_ports() to adapt its behavior
  bool port_monitoring;

  // last-test-was-an-attack flags: useful to keep in mind results of
  // tests if report_only_first_attack==true
  bool last_wmw_test_was_an_attack;
  bool last_ks_test_was_an_attack;
  bool last_pcs_test_was_an_attack;

};

#endif
