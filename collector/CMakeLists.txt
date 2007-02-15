ADD_EXECUTABLE(collector collectorconfobj.cpp collector.cpp collector_main.cpp detectmod.cpp detectmodexporter.cpp manager.cpp modulecontainer.cpp recorder.cpp)
TARGET_LINK_LIBRARIES(collector commonUtils ipfixCollector ${LIBXML2_LIBRARIES} ${XML_BLASTER_C_LIBRARIES} ${XML_BLASTER_CPP_LIBRARIES} ${XERCES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

IF (XML_BLASTER_FOUND)
  INCLUDE_DIRECTORIES(${XML_BLASTER_INCLUDE_DIR})
ENDIF (XML_BLASTER_FOUND)
