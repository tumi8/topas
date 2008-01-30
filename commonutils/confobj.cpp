/**************************************************************************/
/*    Copyright (C) 2005-2007 Lothar Braun <mail@lobraun.de>              */
/*                            Gerhard Muenz                               */
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
{
    if (sourceType == XML_FILE) {
	documentTree = xmlReadFile(filename.c_str(), NULL, 0);
	if (NULL == documentTree) {
	    throw exceptions::XMLException("Could not parse " + filename);
	}

	currentLevel = xmlDocGetRootElement(documentTree);
	// look for CONFIGURATION element and set currentLevel to CONFIGURATION
	while ((NULL != currentLevel) && (currentLevel->type != XML_ELEMENT_NODE) && (!xmlStrEqual(currentLevel->name,
		    (const xmlChar*) config_space::CONFIGURATION.c_str()))) {
	    currentLevel = currentLevel->next;
	}

	if (NULL == currentLevel)
	    throw exceptions::XMLException("Could not find <" + config_space::CONFIGURATION +
		    "> element in file " + filename);
	if (currentLevel->children == NULL)
	    throw exceptions::XMLException("Element <" + config_space::CONFIGURATION +
		    "> is empty in file " + filename);
	currentLevel = currentLevel->children;
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

    savedPosition = currentLevel;
}

XMLConfObj::~XMLConfObj()
{
    if (documentTree) {
	xmlFreeDoc(documentTree);
    }
    xmlCleanupParser();
}

bool XMLConfObj::nodeExists(const std::string& nodeName)
{
    return (findNode(nodeName, true) != NULL);
}

bool XMLConfObj::selectNodeIfExists(const std::string& nodeName)
{
    xmlNodePtr node = findNode(nodeName, true);
    if (node) {
	savedPosition = node;
	return true;
    } else {
	return false;
    }
}

bool XMLConfObj::selectNextNodeIfExists()
{
    xmlNodePtr node = findNode(false);
    if (node) {
	savedPosition = node;
	return true;
    } else {
	return false;
    }
}

bool XMLConfObj::selectNextNodeIfExists(const std::string& nodeName)
{
    xmlNodePtr node = findNode(nodeName, false);
    if (node) {
	savedPosition = node;
	return true;
    } else {
	return false;
    }
}

bool XMLConfObj::nodeIsEmpty()
{
    xmlNodePtr node = savedPosition->children;
    while (node && (node->type != XML_ELEMENT_NODE))
	node = node->next;
    return (node == NULL);
}

void XMLConfObj::enterNode(const std::string& nodeName)
{
    xmlNodePtr node = findNode(nodeName, true);
    if (node) {
	node = node->children;
	while (node && (node->type != XML_ELEMENT_NODE))
	    node = node->next;
	if(node) {
	    savedPosition = currentLevel = node;
	} else {
	    throw exceptions::XMLException("Cannot enter empty node: " + nodeName);
	}
    } else {
	throw exceptions::XMLException("Node does not exist: " + nodeName);
    }
}

bool XMLConfObj::enterNodeIfNotEmpty()
{
    xmlNodePtr node = savedPosition->children;
    while (node && (node->type != XML_ELEMENT_NODE))
	node = node->next;
    if(node) {
	savedPosition = currentLevel = node;
	return true;
    } else {
	return false;
    }
}

void XMLConfObj::leaveNode()
{
    xmlNodePtr node;
    if (NULL != (node = currentLevel->parent)) {
	savedPosition = node;
	while (node) {
	    if (node->type == XML_ELEMENT_NODE)
		currentLevel = node;
	    node = node->prev;
	}
    } else {
	throw exceptions::XMLException("Already at root level");
    }

}

std::string XMLConfObj::getNodeName()
{
    return std::string((const char*)(savedPosition->name));
}

std::string XMLConfObj::getValue()
{
    return std::string((const char*)xmlNodeGetContent(savedPosition));
}

std::string XMLConfObj::getValue(const std::string& nodeName)
{
    xmlNodePtr node = findNode(nodeName, true);
    if (NULL == node) {
	throw exceptions::XMLException("No such node in current level: " + nodeName);
    }
    return std::string((const char*)xmlNodeGetContent(node));
}

bool XMLConfObj::attributeExists(const std::string& attrName)
{
    const char* ret = (const char*)xmlGetProp(savedPosition, (const xmlChar*)attrName.c_str());
    return (NULL != ret);
}

std::string XMLConfObj::getAttribute(const std::string& attrName)
{
    const char* ret = (const char*)xmlGetProp(savedPosition, (const xmlChar*)attrName.c_str());
    if (NULL == ret) {
	throw exceptions::XMLException("No attribute \"" + attrName + "\" in current node");
    }
    return ret;
}

std::string XMLConfObj::getAttribute(const std::string& nodeName, const std::string& attrName)
{
    xmlNodePtr node = findNode(nodeName, true);
    if (NULL == node) {
	throw exceptions::XMLException("No such node in current level: " + nodeName);
    }
    const char* ret = (const char*)xmlGetProp(node, (const xmlChar*)attrName.c_str());
    if (NULL == ret) {
	throw exceptions::XMLException("No attribute \"" + attrName + "\" in node \"" + nodeName + "\"");
    }
    return ret;
}

xmlNodePtr XMLConfObj::findNode(bool first)
{
    xmlNodePtr node;
    if(first)
	node = currentLevel;
    else
	node = savedPosition->next;
    while (node && (node->type != XML_ELEMENT_NODE)) {
	node = node->next;
    }
    return node;
}

xmlNodePtr XMLConfObj::findNode(const std::string& nodeName, bool first)
{
    xmlNodePtr node;
    if(first)
	node = currentLevel;
    else
	node = savedPosition->next;
    while (node && ((node->type != XML_ELEMENT_NODE) || (!xmlStrEqual(node->name, (const xmlChar*) nodeName.c_str())))) {
	node = node->next;
    }
    return node;
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
