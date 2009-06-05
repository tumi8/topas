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

#ifndef _CONF_OBJ_H_
#define _CONF_OBJ_H_

#include "global.h"
#include "exceptions.h"


#include <libxml/parser.h>
#include <libxml/tree.h>

#include <string.h>

#include <string>
#include <vector>


/**
 * Supports configuration file using XML-Files. This class is intended to substitute the old
 * ConfObj class which was used to parser ini-Files.
 */
class XMLConfObj {
public:
	
	typedef enum { XML_FILE, XML_STRING } XmlType;

	/**
	 * Constructor reads and parses the given XML-File or XML-String.
	 * @param filename Associated XML-File or XML-String 
	 * @param type XML_FILE or XML_STRING
	 */
	XMLConfObj(const std::string& filename, XmlType sourceType);

	/**
	 * Destructor ...
	 */
	~XMLConfObj();

	/**
	 * Checks if given node exists on the current level.
	 * @param nodeName name of node to look for.
	 * @return True if node exists on current leven, false otherwise.
	 */
	bool nodeExists(const std::string& name);

	/**
	 * Selects the first occurence of node with given name on the current level.
	 * Returns false if node does not exist.
	 * @param nodeName
	 * @return True if node exists
	 */
	bool selectNodeIfExists(const std::string& nodeName);

	/**
	 * Selects the next node on the current level.
	 * Returns false if node does not exist.
	 * @param nodeName
	 * @return True if node exists
	 */
	bool selectNextNodeIfExists();

	/**
	 * Selects the next occurence of node with given name on the current level.
	 * Returns false if node does not exist.
	 * @param nodeName
	 * @return True if node exists
	 */
	bool selectNextNodeIfExists(const std::string& nodeName);

	/**
	 * Checks if the current node is empty.
	 * @return True if node is empty.
	 */
	bool nodeIsEmpty();

	/**
	 * Enters the first occurence of a node with given name. All following 
	 * calls to methods within this class will be relative to this node. 
	 * Use @c leaveNode() to get back to the former level.
	 * Throws an exeption if node does not exist or if it is empty.
	 * @param nodeName
	 */
	void enterNode(const std::string& nodeName);

	/**
	 * Enters the current node. All following calls to methods
	 * within this class will be relative to this node. Use @c leaveNode()
	 * to get back to the former level.
	 * Returns false if node is empty.
	 * @param nodeName
	 * @return True if node is not empty.
	 */
	bool enterNodeIfNotEmpty();

	/**
	 * Leaves the current level within the XML-Tree.
	 * Throws exceptions::XMLException, if we are already on document root.
	 */
	void leaveNode();

	/**
	 * Gets the name of the current node.
	 * Note: The result is "text" if the node is a text node (not element).
	 * @return Node name or empty string.
	 */
	std::string getNodeName();

	/**
	 * Gets value of the current node.
	 * @return Extracted value from given node.
	 */
	std::string getValue();

	/**
	 * Gets value from given node. Throws exceptions::XMLException if node
	 * does not exist on current level.
	 * Note: This function does not change the selected node (savedPosition).
	 * @param nodeName name of node to extract value from.
	 * @return Extracted value from given node.
	 */
	std::string getValue(const std::string& nodeName);

	/**
	 * Checks if the current node has the given attribute.
	 * @return True if attribute exists.
	 */
	bool attributeExists(const std::string& attrName);

	/**
	 * Gets given attribute from current Node. Throws exceptions::XMLException if
	 * node does not exists on the current level or if node does not have the given attribute.
	 * @param attrName Name of attribute to extract.
	 * @return Value of given attribute.
	 */
	std::string getAttribute(const std::string& attrName);

	/**
	 * Gets given attribute from given Node. Throws exceptions::XMLException if
	 * node does not exists on the current level or if node does not have the given attribute.
	 * Note: This function does not change the selected node (savedPosition).
	 * @param nodeName Name of node to look for.
	 * @param attrName Name of attribute to extract.
	 * @return Value of given attribute.
	 */
	std::string getAttribute(const std::string& nodeName, const std::string& attrName);

	/**
	 * Returns xml data as a string
	 * @return String representation of xml data
	 */
	std::string toString();

private:
	xmlDocPtr documentTree;
	xmlNodePtr currentLevel;
	xmlNodePtr savedPosition;

	/**
	 * Searches node on current level
	 * @param nodeName
	 * @param first	true to search from currentLevel->next, false to search from savedPosition->next
	 * @return Pointer to node or NULL if node does not exists.
	 */
	xmlNodePtr findNode(const std::string& nodeName, bool first);

	/**
	 * Searches (next) node on current level
	 * @param first	true to search from currentLevel->next, false to search from savedPosition->next
	 * @return Pointer to node or NULL if node does not exists.
	 */
	xmlNodePtr findNode(bool first);
};


/**
 * @deprecated
 * Compability class. This will enable XML-File configuration in code that uses the
 * old ini-file interface. The interface is only capable of flat XML-Trees. The class
 * enables using XML files with little changes to old code (you have to declare an ConfObjX isteed of ConfObj).
 * Use @c XMLConfObj in new code.
 */ 
class ConfObj {
 public:
        /** 
         * Constructor...
         * @param file XML-file which should be parsed.
         */
        ConfObj(const std::string& file) : xmlObj(file, XMLConfObj::XML_FILE) {}

        /**
         * Destructor....
         */
	~ConfObj() {
		/* deletes allocated memory for the chars* from getValue etc*/
		typedef std::vector<char*>::iterator IT;
		for (IT i = allocated.begin(); i != allocated.end(); ++i) {
			delete *i;
		}
	}

		

        /**
         * Gets value assigned to entry in section.
         * @param section Section to search the entry in.
         * @param entry Entry to look for
         * @return Value assigned to the entry, empty string if value is not in config file
         */
        char* getValue(const std::string& section, const std::string& entry) {
		char* ret = NULL;
		try {
			xmlObj.enterNode(section);
		} catch(exceptions::XMLException& e) { 
			return ret; 
		}
		try {
			std::string tmp = xmlObj.getValue(entry);
			ret = new char[tmp.size() + 1];
			strcpy(ret, tmp.c_str());
			allocated.push_back(ret);
		} catch (exceptions::XMLException& e){
		}
		xmlObj.leaveNode();
		return ret;
	}

        /**
         * Gets value assigned to entry from an previously set section (@c setSection()).
         * @param entry Entry to look for
         * @return Value assigned to the entry, empty string if value is not in config file or
         * section was not specified using setSection() before.
         */
        char* getValue(const std::string& entry) {
		return getValue(s, entry);
	}

        /**
         * Checks if given section exists in associated config file.
         * @param section Section to look for
         * @return 1 if entry exists, 0 otherwise
         */
        int findSection(const std::string& section) {
		return xmlObj.nodeExists(section);
	}
        
        /**
         * Sets section to search in on next call to @c geValue()
         * @pararm section Section to look in
         */
        void setSection(const std::string& section) {
		s = section;
	}
private:
	XMLConfObj xmlObj;
	std::string s;
	std::vector<char*> allocated;
};

#endif 
