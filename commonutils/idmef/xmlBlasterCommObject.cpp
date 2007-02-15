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

#include "xmlBlasterCommObject.h"

XmlBlasterCommObject::XmlBlasterCommObject(Global& glob) 
	: ME("XmlBlasterCommObject"),
	  global_(glob),
	  con(glob),
	  log_(glob.getLog())
{
	log_.info(ME, "Trying to connect to xmlBlaster with C++ client lib " + Global::getReleaseId() +
		  " from " + Global::getBuildTimestamp());
}

XmlBlasterCommObject::~XmlBlasterCommObject() 
{
};

void XmlBlasterCommObject::connect()
{
	/* 
	 * Used to initialize the failsafe behaviour of the client.
	 * No failsafe behaviour implemented yet
	 */
	con.initFailsafe(this);
	/* Creates a connect qos where user and password fields ar empty  */
	ConnectQos qos(global_, "", "");
	log_.info(ME, "connecting to xmlBlaster...");
	/* 
	 * Connects to xmlBlaster and gives a pointer to this class to tell
	 * which update method to invoke when callbacks come from the server
	 */
	ConnectReturnQos retQos = con.connect(qos, this);
	log_.info(ME, "successfully connected to xmlBlaster");
};

void XmlBlasterCommObject::disconnect()
{
	/* disconnects from xmlBlaster */
	DisconnectQos disconnectQos(global_);                                             
	con.disconnect(disconnectQos);
	log_.info(ME, "disconnected from xmlBlaster");
};

void XmlBlasterCommObject::subscribe(const std::string& key, SubscriptionType subType)
{
	/* Minimal constructor */
	SubscribeKey subKey(global_);
	/*
	 * Subscribe key. By invoking setOid you implicitly choose the 'EXACT' mode.      
	 * If you want to subscribe with XPATH use setQueryString instead 
	 */
	if (subType == XPATH) {
		subKey.setQueryString(key);
	} else if (subType == MESSAGE) {
		subKey.setOid(key);
	} else {
		log_.error(ME, "unknown subsription type: " + subType); 
	}
	SubscribeQos subQos(global_);                                                     
	log_.info(ME, "subscribing to xmlBlaster with key: \"" + subKey.toXml() + "\"");
	SubscribeReturnQos subRetQos = con.subscribe(subKey, subQos);                     
	log_.info(ME, "successfully subscribed to xmlBlaster");
};

void XmlBlasterCommObject::publish(const std::string& message, const std::string& topic)
{
	PublishQos publishQos(global_);                                                   
	PublishKey publishKey(global_);                                                   
	publishKey.setOid(topic);                                                 
	MessageUnit msgUnit(publishKey, message, publishQos);                        
	PublishReturnQos pubRetQos = con.publish(msgUnit);                                
	log_.trace(ME, "successfully published to xmlBlaster");        
};

void XmlBlasterCommObject::erase(const std::string& key)
{
	EraseKey eraseKey(global_);                                                       
	eraseKey.setOid(key);                                                   
	EraseQos eraseQos(global_);                                                       
	log_.info(ME, "erasing the published message. Key: " + eraseKey.toXml()); 
	std::vector<EraseReturnQos> eraseRetQos = con.erase(eraseKey, eraseQos);               
	for (size_t i=0; i < eraseRetQos.size(); i++ ) {                                  
		log_.info(ME, "successfully erased the message.");                                         
	}                   
};

std::string XmlBlasterCommObject::update(const std::string& sessionId, UpdateKey& updateKey,
					 const unsigned char* content,
					 long contentSize, UpdateQos& updateQos)
{
	mutex.lock();
	updateMessage = std::string((char*)content, (char*)(content) + contentSize);
	updateAvailable = true;
	mutex.unlock();
	return "";
};

std::string XmlBlasterCommObject::getUpdateMessage()
{
	mutex.lock();
	std::string copy = "";
	if (updateAvailable) {
		copy = updateMessage;
		updateAvailable = false;
	}
	mutex.unlock();
	return copy;
}

bool XmlBlasterCommObject::reachedAlive(StatesEnum /*oldState*/, I_ConnectionsHandler* /*connectionsHandler*/)
{
	log_.warn(ME, "reconnected");
	return true;
}

void XmlBlasterCommObject::reachedDead(StatesEnum /*oldState*/, I_ConnectionsHandler* /*connectionsHandler*/)
{
	log_.warn(ME, "lost connection");
}

void XmlBlasterCommObject::reachedPolling(StatesEnum /*oldState*/, I_ConnectionsHandler* /*connectionsHandler*/)
{
	log_.warn(ME, "going to poll modus");
}

bool XmlBlasterCommObject::isConnected()
{
	return con.isAlive();
}

#endif //IDMEF_SUPPORT_ENABLED
