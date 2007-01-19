ADD_LIBRARY(commonUtils confobj.cpp exceptions.cpp mutex.cpp packetstats.cpp
sharedobj.cpp metering.cpp idmef/idmefmessage.cpp
idmef/xmlBlasterCommObject.cpp)

IF (XML_BLASTER_FOUND)
  INCLUDE_DIRECTORIES(${XML_BLASTER_INCLUDE_DIR})
ENDIF (XML_BLASTER_FOUND)
