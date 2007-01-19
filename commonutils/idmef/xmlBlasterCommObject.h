/**************************************************************************/
/*    Copyright (C) 2007 Raimondas Sasnauskas <sasnausk@informatik.uni-tuebingen.de>  */
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
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA  */
/*                                                                        */
/**************************************************************************/

#ifdef IDMEF_SUPPORT_ENABLED

#include <client/XmlBlasterAccess.h>
#include <util/Global.h>

#include <string>
#include <vector>

using namespace org::xmlBlaster::util;
using namespace org::xmlBlaster::util::qos;
using namespace org::xmlBlaster::util::dispatch;
using namespace org::xmlBlaster::client;
using namespace org::xmlBlaster::client::qos;
using namespace org::xmlBlaster::client::key;

class XmlBlasterCommObject : public I_Callback,
			     public I_ConnectionProblems
{
public: 
	XmlBlasterCommObject(Global& glob);
	virtual ~XmlBlasterCommObject();

	bool reachedAlive(StatesEnum /*oldState*/, I_ConnectionsHandler* /*connectionsHandler*/);
	
	void reachedDead(StatesEnum /*oldState*/, I_ConnectionsHandler* /*connectionsHandler*/);

	void reachedPolling(StatesEnum /*oldState*/, I_ConnectionsHandler* /*connectionsHandler*/);

	bool isConnected();

	/*
	 * Establish a connection
	 */
	void connect();

        /**
         * Tear down the connection and clean up
         */
	void disconnect();

	/**
         * Subscribe key
	 * @param key Topic or XPath expression
	 * @param xpath "true" for XPath expression, "false" otherwise
         */	
	void subscribe(const std::string& key, bool useXPath);

	/**
         * Publish a message with the given topic
	 * @param message Message to publish
	 * @param topic Topic
         */
	void publish(const std::string& message, const std::string& topic);

	/**
         * Erase the published message
	 * @param key Message key to erase
         */
	void erase(const std::string& key);

	/**
	 * Callbacks from xmlBlaster arrive here 
	 */
	std::string update(const std::string& sessionId, UpdateKey& updateKey,
			   const unsigned char* content,
			   long contentSize, UpdateQos& updateQos);

private:
	Global& global_;
	/* xmlBlaster accessor */
	XmlBlasterAccess con;
	/* The string identifying this class when logging */
	std::string ME;
	/* The reference to the log object for this instance */
	I_Log& log_;
};

#endif //IDMEF_SUPPORT_ENABLED
