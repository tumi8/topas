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

#ifndef _CONF_OBJ_H_
#define _CONF_OBJ_H_

#include "global.h"
#include "exceptions.h"


#include <libxml/parser.h>
#include <libxml/tree.h>


#include <string>
#include <vector>


/**
 * Supports configuration file using XML-Files. This class is intended to substitute the old
 * ConfObj class which was used to parser ini-Files.
 */
class XMLConfObj {
public:
	/**
	 * Constructor reads and parses the given XML-File.
	 * @param filename Associated XML-File
	 */
	XMLConfObj(const std::string& filename);

	/**
	 * Destructor ...
	 */
	~XMLConfObj();

	void setNode(const std::string& nodeName);

	/**
	 * Enters node within the XML-Document. All following calls to methods
	 * within this class will be relative to this node. Use @c leaveNode()
	 * to get back to the former level.
	 * Throws  exceptions::XMLException, if node does not exist.
	 * @param nodeName
	 */
	void enterNode(const std::string& nodeName);

	/**
	 * Enters next node in the current level with node name passed to
	 * @c enterNode(), @c getValue() or @c setNode().
	 */
	void enterNextNode();

	/**
	 * Leaves the current level within the XML-Tree.
	 * Throws exceptions::XMLException, if we are already on document root.
	 */
	void leaveNode();

	/**
	 * Checks if given node exists on the current level.
	 * @param nodeName name of node to look for.
	 * @return True if node exists on current leven, false otherwise.
	 */
	bool nodeExists(const std::string& name);

	/**
	 * Gets value from given node. Throws exceptions::XMLException if node
	 * does not exist on current level.
	 * @param nodeName name of node to extract value from.
	 * @return Extracted value from given node.
	 */
	std::string getValue(const std::string& nodeName);

	/**
	 *  Tries to find the next node in the current level with the name passed to the last
	 *  call to @c getValue() or @c setNode(). 
	 *  @return true if such an node exists, false otherwise
	 */
	bool nextNodeExists();

	/**
	 * Tries to find more nodes with the name passed to the last call to @c getValue().
	 * Throws @c XMLException when no next node exists (either because @c getValue() was
	 * not called before or becase we already iterated through all existing nodes with the same
	 * name.
	 * @return Extracted value from next node.
	 */
	std::string getNextValue();
	
	/**
	 * Gets given attribute from given Node. Throws exceptions::XMLException if
	 * node does not exists on the current level or if node does not have the given attribute.
	 * @param nodeName Name of node to look for.
	 * @param attrName Name of attribute to extract.
	 * @return Value of given attribute.
	 */
	std::string getAttribute(const std::string& nodeName, const std::string& attrName);

private:
	xmlDocPtr documentTree;
	xmlNodePtr currentLevel;
	xmlNodePtr savedPosition;
	/* FIXME: remove this diry hack */
	bool enterCurrentSaved;

	/**
	 * Searches node on current level
	 * @param nodeName
	 * @return Pointer to node or NULL if node does not exists.
	 */
	xmlNodePtr findNode(const std::string& nodeName);
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
        ConfObj(const std::string& file) : xmlObj(file) {}

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
