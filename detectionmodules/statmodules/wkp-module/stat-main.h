/**************************************************************************/
/*    Copyright (C) 2006-07                                               */
/*    Romain Michalec, Sven Wiebusch, Gerhard Muenz                       */
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

#ifndef _STAT_MAIN_H_
#define _STAT_MAIN_H_

#include "stat-store.h"
#include "shared.h"
#include "params.h"
#include <detectionbase.h>
#include <sstream>
#include <algorithm> // sort(...), unique(...)


// ========================== CLASS Stat ==========================

// constants for metrics
enum Metric {
    BYTES_IN = 0,
    BYTES_OUT = 1,
    PACKETS_IN = 2,
    PACKETS_OUT = 3,
    RECORDS_IN = 4,
    RECORDS_OUT = 5,
    BYTES_IN_PER_PACKET_IN = 6,
    BYTES_OUT_PER_PACKET_OUT = 7,
    BYTES_IN_PER_RECORD_IN = 8,
    BYTES_OUT_PER_RECORD_OUT = 9,
    PACKETS_IN_PER_RECORD_IN = 10,
    PACKETS_OUT_PER_RECORD_OUT = 11,
    BYTES_OUT_MINUS_BYTES_IN = 12,
    PACKETS_OUT_MINUS_PACKETS_IN = 13,
    RECORDS_OUT_MINUS_RECORDS_IN = 14,
    BYTES_IN_MINUS_BYTES_OUT = 15,
    PACKETS_IN_MINUS_PACKETS_OUT = 16,
    RECORDS_IN_MINUS_RECORDS_OUT = 17,
    NUMBER_OF_METRICS = 18
};

// main class: does tests, stores samples and params,
// reads and stores test parameters...

class Stat
#ifdef OFFLINE_ENABLED
: public DetectionBase<StatStore, OfflineInputPolicy<StatStore> >
#else
: public DetectionBase<StatStore>
#endif
{

    public:
	struct MetricData {
	    Metric type;
	    bool absolute;
	    bool negate;
	    bool applyQuasiDifference;
	    float factor;
	    std::string name;
	};

	typedef std::vector<MetricData> MetricList;

	Stat(const std::string & configfile);
	~Stat();

	/* Test function. This function will be called in periodic intervals.
	 *   Set interval size with @c setAlarmTime().
	 * @param store : Pointer to a storage object of class StatStore containing
	 *   all data collected since last call to test. This pointer points to
	 *   your memory. That means that you may store the pointer.
	 *   IMPORTANT: _YOU_ have to free the object's memory when you don't need
	 *   it any longer (use delete-operator to do that). */
	void test(StatStore * store);

#ifdef IDMEF_SUPPORT_ENABLED
	/**
	 * Update function. This function will be called, whenever a message
	 * for subscribed key is received from xmlBlaster.
	 * @param xmlObj Pointer to data structure, containing xml data
	 *               You have to delete the memory allocated for the object.
	 */
	void update(XMLConfObj* xmlObj);
#endif


    private:

	// signal handlers
	static void sigTerm(int);
	static void sigInt(int);

	// this function is called by the Stat constructor, its job is to extract
	// user's preferences and test parameters from the XML config file:
	void init(const std::string & configfile);

	// as the init function is really huge, we divide it into tasks:
	// those related to the user's preferences regarding the module...
	void init_endpoints(XMLConfObj *);
	void init_cusum_test(XMLConfObj *);
	void init_wkp_test(XMLConfObj *);

	// create string with full configuration
	std::string config_overview();

	// the following functions are called by the test()-function:
	int64_t get_metric (const Info &, Metric );
	std::vector<int64_t> extract_data (const Info &, const Info &);
	std::vector<int64_t> extract_pca_data (Params &, const Info &, const Info &);
	void wkp_update(Params &, const std::vector<int64_t> &);
	void wkp_test(Params &);
	void cusum_update(Params &, const std::vector<int64_t> &);
	void cusum_test(Params &);

	// these functions are called by extract_pca_data() to calculate ...
	// ... a single entry of the covariance matrix
	double covariance (const double &, const double &, const double &);
	// ... the standard deviation of a single metric
	double standard_deviation (const double &, const double &);

	// this function is called by wkp_test() and cusum_test() to extract a
	// single metric to test from a std::vector<int64_t>
	std::list<int64_t> getSingleMetric(const std::list<std::vector<int64_t> > &, const short &);

	// the following functions are called by the wkp_test()-function
	double stat_test_wmw(std::list<int64_t> &, std::list<int64_t> &, bool &, std::string);
	double stat_test_ks (std::list<int64_t> &, std::list<int64_t> &, bool &, std::string);
	double stat_test_pcs(std::list<int64_t> &, std::list<int64_t> &, bool &, std::string);

	// File where data will be stored to (in ONLINE MODE)
	// or be read from (in OFFLINE MODE)
	std::string offlineFile;

	// If the user specifies an output_dir, this flag will be set to true
	// and output files (for test-params and metrics) will be generated and
	// stored into the output_dir
	bool createFiles;
	std::string output_dir;

	// Map that holds all the parameters for each endpoint
	std::map<EndPoint, Params> EndpointParams;

#ifdef IDMEF_SUPPORT_ENABLED
	// IDMEF-Message
	IdmefMessage idmefMessage;
#endif

	// logging stream (for logging into file)
	MsgStream logStr;

	// user's preferences (defined in the XML config file):
	// holds the constants for the (splitted) metrics
	MetricList metrics;
	int noise_threshold_packets;
	int noise_threshold_bytes;
	int stat_test_frequency;
	bool report_only_first_attack;
	short pause_update_when_attack;
	// fix PCA stuff
	bool use_pca;
	int pca_learning_phase;
	bool use_correlation_matrix;


	// test parameters (defined in the XML config file):

	// for cusum-test
	bool enable_cusum_test;
	double amplitude_percentage;
	bool log_variance_ewma;
	uint16_t repetition_factor;
	uint16_t cusum_learning_phase;
	double smoothing_constant;

	// for wkp-tests
	bool enable_wmw_test;
	bool enable_ks_test;
	bool enable_pcs_test;
	bool enable_wkp_test; // summarizes the above three
	uint16_t sample_old_size;
	uint16_t sample_new_size;
	bool wmw_two_sided;
	double significance_level;


	// the following counter enables us to call a statistical test every
	// X=stat_test_frequency times that the test() method is called,
	// rather than everytime
	int test_counter;

	std::ofstream storefile;
};


#endif
