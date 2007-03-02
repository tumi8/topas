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

#include "confobj.h"


#include <concentrator/msg.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <cstring>


/* FIXME: remove all the dirty hacks!!!!! */

XMLConfObj::XMLConfObj(const std::string& filename, XmlType sourceType)
	: documentTree(NULL), savedPosition(NULL), enterCurrentSaved(false)
{
	if (sourceType == XML_FILE) {
		documentTree = xmlReadFile(filename.c_str(), NULL, 0);
		if (NULL == documentTree) {
			throw exceptions::XMLException("Could not parse " + filename);
		}

		currentLevel = xmlDocGetRootElement(documentTree);
		// look for CONFIGURATION element and set currentLevel to CONFIGURATION
		while (NULL != currentLevel && !xmlStrEqual(currentLevel->name,
							    (const xmlChar*) config_space::CONFIGURATION.c_str())) {
			currentLevel = currentLevel->next;
		}

		if (NULL == currentLevel) {
			throw exceptions::XMLException("Could not find <" + config_space::CONFIGURATION +
						       "> element in file " + filename);
		}
	} else if (sourceType == XML_STRING) {
		documentTree  = xmlReadMemory(filename.c_str(), filename.size(), "noname.xml", NULL, 0);
		if (NULL == documentTree) {
			throw exceptions::XMLException("Could not parse " + filename);
		} else {
			currentLevel = xmlDocGetRootElement(documentTree);
		}
	} else {
		throw exceptions::XMLException("Unknown source type " + sourceType);
	}
}

XMLConfObj::~XMLConfObj()
{
	if (documentTree) {
		xmlFreeDoc(documentTree);
	}
        xmlCleanupParser();
}


void XMLConfObj::setNode(const std::string& nodeName)
{
	if (NULL != findNode(nodeName)) {
		savedPosition = findNode(nodeName);
		enterCurrentSaved = true;
	} else {
		throw exceptions::XMLException("No such node in current level: " + nodeName);
	}
}

void XMLConfObj::enterNode(const std::string& nodeName)
{
	if (NULL != findNode(nodeName)) {
		currentLevel = findNode(nodeName);
		savedPosition = NULL;
	} else {
		throw exceptions::XMLException("No such node in current level: " + nodeName);
	}
}

void XMLConfObj::enterNextNode()
{
	if (!nextNodeExists()) {
		throw exceptions::XMLException("No next node");
	}
	if (enterCurrentSaved) {
		currentLevel = savedPosition;
		enterCurrentSaved = false;
	} else {
		xmlNodePtr next = savedPosition->next;
		while (next && xmlStrEqual(next->name, (const xmlChar*)"text")) {
			next = next->next;
		}
		currentLevel = next;
	}

	savedPosition = NULL;
}

void XMLConfObj::leaveNode()
{
	if (NULL != currentLevel->parent) {
		savedPosition = currentLevel;
		currentLevel = currentLevel->parent;
		enterCurrentSaved = false;
	} else {
		throw exceptions::XMLException("Already at root level");
	}

}

bool XMLConfObj::nodeExists(const std::string& nodeName)
{
	if (NULL != findNode(nodeName)) {
		return true;
	}

	return false;
}

std::string XMLConfObj::getValue(const std::string& nodeName)
{
	xmlNodePtr node = findNode(nodeName);
	if (NULL == node) {
		throw exceptions::XMLException("No such node in current level: " + nodeName);
	}
	savedPosition = node;
	return std::string((const char*)xmlNodeGetContent(node));
}

std::string XMLConfObj::getAttribute(const std::string& nodeName, const std::string& attrName)
{
	xmlNodePtr node = findNode(nodeName);
	if (NULL == node) {
		throw exceptions::XMLException("No such node in current level: " + nodeName);
	}
	const char* ret = (const char*)xmlGetProp(node, (const xmlChar*)attrName.c_str());
	if (NULL == ret) {
		throw exceptions::XMLException("No attribute \"" + attrName + "\" in node \"" + nodeName + "\"");
	}
	return ret;
}

xmlNodePtr XMLConfObj::findNode(const std::string& nodeName)
{
	xmlNodePtr node = currentLevel->xmlChildrenNode;
	while (node != NULL) {
		if (xmlStrEqual(node->name, (const xmlChar*) nodeName.c_str())) {
			return node;   
		}
		node = node->next;
	}
	return node;
}

bool XMLConfObj::nextNodeExists()
{
	xmlNodePtr tmp;
	if (enterCurrentSaved)
		tmp = savedPosition;
	else
		tmp = savedPosition->next;
	while (tmp && xmlStrEqual(tmp->name, (const xmlChar*)"text")) {
		tmp = tmp->next;
	}
	if (tmp)
		return true;

	return false;
}

std::string XMLConfObj::getNextValue()
{
	if (!nextNodeExists())
		throw exceptions::XMLException("No next node exists");
	savedPosition = savedPosition->next;
	while (savedPosition && xmlStrEqual(savedPosition->name, (const xmlChar*)"text")) {
		savedPosition = savedPosition->next;
	}
	return std::string((const char*)xmlNodeGetContent(savedPosition));
}

std::string XMLConfObj::toString()
{
	xmlIndentTreeOutput = 1;
        xmlKeepBlanksDefault(0);
        xmlBufferPtr xmlBufPtr = xmlBufferCreate();
        xmlNodeDump(xmlBufPtr, documentTree, currentLevel, 0, 1);
        std::string ret = std::string((char *)xmlBufPtr->content, xmlBufPtr->use);
        xmlBufferFree(xmlBufPtr);
	return ret;
}
