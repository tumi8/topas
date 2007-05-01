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

#ifndef _DETECT_CALLBACKS_H_
#define _DETECT_CALLBACKS_H_


#include <concentrator/rcvIpfix.h>
//#include <concentrator/msg.h>


/**
 * Will be called whenever a new template with SetId 2 arrives.
 * @param handle Control structure
 * @param sourceID SourceID of sending monitoring station
 * @param ti Struct containing information about the received template
 */
template <class PacketReader, class Storage>
int newTemplateArrived(void* handle,  SourceID sourceID, TemplateInfo* ti) 
{
        return 0;
}



/**
 * Will be called whenever a record from an incoming IPFIX-Packet was extracted.
 * @param handle control structure, responsible for further handling of the data
 * @param sourceID Source ID of sending monitoring station
 * @param length Length of received data set
 * @param data Received data set
 */
template <class PacketReader, class Storage>
int newDataRecordArrived(void* handle, SourceID sourceID, TemplateInfo* ti,
                            uint16_t length, FieldData* data) 
{
        PacketReader* input = static_cast<PacketReader*>(handle);
        input->recordMutex.lock();
        Storage* buf;
        if(buf = input->getBuffer()) {
		if (buf->recordStart(sourceID)) {
			buf->setValid(true);
			for (int i = 0; i < ti->fieldCount; ++i) {
				if (input->isIdInList(ti->fieldInfo[i].type.id)) {
					buf->addFieldData(ti->fieldInfo[i].type.id, data + ti->fieldInfo[i].offset,
							ti->fieldInfo[i].type.length, ti->fieldInfo[i].type.length);
				}
			}
			buf->recordEnd();
		}
	}/* Error message is printed in filepolicy.h with an error counter
	else { 
		msg(MSG_ERROR, "DetectionBase: getBuffer() returned NULL, record dropped!");
	}*/
        input->recordMutex.unlock();
        return 0;
}

/**
 * Will be called whenever a buffered template with Set Id 2 expired.
 * @param handle Control structure, responsible for further handling of event
 * @param sourceID Source ID of the sending montioring station
 * @param ti Struct containing information about the expired template
 */
template <class PacketReader, class Storage>
int templateDestroyed(void* handle, SourceID sourceID, TemplateInfo* ti) 
{
        return 0;
}


/**
 * Will be called whenever a new template with SetID 3 arrives.
 * @param handle Control structure, responsible for further handling of incoming template
 * @param sourceID Source ID of sending monitoring station
 * @param optionsTemplateInfo Struct containing information about the incoming template
 */
template <class PacketReader, class Storage>
int newOptionsTemplateArrived(void* handle, SourceID sourceID, OptionsTemplateInfo* optionsTemplateInfo) 
{
        return 0;
}

/**
 * Will be called whenever a set of option records was exctrated
 * @param handle Control structure, responsible for further handling of the options records.
 * @param sourceID Source ID of sending monitoring station
 * @param oti Struct containing information about the corresponding options template
 * @param length Length of received options template data record set
 * @param data Options template data record
 */
template <class PacketReader, class Storage>
int newOptionRecordArrived(void* handle, SourceID sourceID, OptionsTemplateInfo* oti,
                              uint16_t length, FieldData* data) 
{
        return 0;
}

/**
 * Will be call whenever a options template expired.
 * @param handle Control structure, responsible for further handling of the event.
 * @param sourceID Source ID of sending montitoring station.
 * @param optionsTemplateInfo Struct, containing information about the expired template.
 */
template <class PacketReader, class Storage>
int optionsTemplateDestroyed(void* handle, SourceID sourceID, OptionsTemplateInfo* optionsTemplateInfo) 
{

        return 0;
}


/**
 * Will be called whenever a new template with SetId 4 arrives.
 * @param handle Control structure
 * @param sourceID SourceID of sending monitoring station
 * @param dataTemplateInfo Struct containing information about the received template
 */
template <class PacketReader, class Storage>
int newDataTemplateArrived(void* handle, SourceID sourceID, DataTemplateInfo* dataTemplateInfo) 
{
        return 0;
}

/**
 * Will be called whenever a record with fixed fields was extracted from an incoming IPFIX-Packet.
 * @param handle control structure, responsible for further handling of the data
 * @param sourceID Source ID of sending monitoring station
 * @param length Length of received the data set
 * @param data Received data set
 */
template <class PacketReader, class Storage> 
int newDataRecordFixedFieldsArrived(void* handle, SourceID sourceID,
                                         DataTemplateInfo* ti, uint16_t length,
                                         FieldData* data) 
{

        /* same as with new_data_record_arrived */
        PacketReader* input = static_cast<PacketReader*>(handle);
        input->recordMutex.lock();
        Storage* buf;
        if(buf = input->getBuffer()) {
		if (buf->recordStart(sourceID)) {
			buf->setValid(true);
			for (int i = 0; i < ti->fieldCount; ++i) {
				if (input->isIdInList(ti->fieldInfo[i].type.id)) {
					buf->addFieldData(ti->fieldInfo[i].type.id, data + ti->fieldInfo[i].offset,
							ti->fieldInfo[i].type.length, ti->fieldInfo[i].type.eid);
				}
			}

			/* pass fixed fields now */
			for (int i = 0; i < ti->dataCount; ++i) {
				if (input->isIdInList(ti->dataInfo[i].type.id)) {
					buf->addFieldData(ti->dataInfo[i].type.id, ti->data +ti->dataInfo[i].offset,
							ti->dataInfo[i].type.length, ti->fieldInfo[i].type.eid);
				}
			}
			buf->recordEnd();
		}
	}/* Error message is printed in filepolicy.h with an error counter
	else { 
		msg(MSG_ERROR, "DetectionBase: getBuffer() returned NULL, record dropped!");
	}*/
        input->recordMutex.unlock();
        return 0;
}

/** 
 * Will be called whenever a buffered template with Set Id 2 expired.
 * @param handle Control structure, responsible for further handling of event
 * @param sourceID Source ID of the sending montioring station
 * @param ti Struct containing information about the expired template
 */
template <class PacketReader, class Storage>
int dataTemplateDestroyed(void* handle, SourceID sourceID, DataTemplateInfo* dataTemplateInfo) 
{
        return 0;
}

#endif
