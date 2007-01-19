FIND_PATH(
  XML_BLASTER_INCLUDE_DIR
  client/XmlBlasterAccess.h util/Global.h 
  /usr/include/ /usr/local/include/
  ${XML_BLASTER_HOME}
  ${XML_BLASTER_HOME}/src/c++/
  )

FIND_LIBRARY(
  XML_BLASTER_C_LIBRARIES
  NAMES xmlBlasterClientCD
  PATHS /usr/lib/ /usr/local/lib/
  ${XML_BLASTER_HOME}
  ${XML_BLASTER_HOME}/lib
  )

FIND_LIBRARY(
  XML_BLASTER_CPP_LIBRARIES
  NAMES xmlBlasterClientD
  PATHS /usr/lib/ /usr/local/lib/
  ${XML_BLASTER_HOME}
  ${XML_BLASTER_HOME}/lib
  )


IF (XML_BLASTER_INCLUDE_DIR AND XML_BLASTER_C_LIBRARIES AND XML_BLASTER_CPP_LIBRARIES)
  SET(XML_BLASTER_FOUND TRUE)
ENDIF (XML_BLASTER_INCLUDE_DIR AND XML_BLASTER_C_LIBRARIES AND XML_BLASTER_CPP_LIBRARIES)

IF (XML_BLASTER_FOUND)
  message(STATUS "Found xmlBlaster")
ELSE (XML_BLASTER_FOUND)
  message(FATAL_ERROR "Could not find xmlBlaster. Disable IDMEF-support with -DIDMEF=OFF or set -DXML_BLASTER_HOME=/path/to/your/xmlBlaster. If you supplied the right path and still get this message: Hurray. You probably found the xmlBlaster easter egg. \"./build all\" does NOT build everything. Go back into your xmlBlaster-directory and execute \"./build -DXMLBLASTER_COMPILE_LOG4CPLUS_PLUGIN=1 cpp-lib\". Then try again.")
ENDIF(XML_BLASTER_FOUND)
