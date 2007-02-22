ADD_EXECUTABLE(collector collectorconfobj.cpp collector.cpp collector_main.cpp detectmod.cpp detectmodexporter.cpp manager.cpp modulecontainer.cpp recorder.cpp)

IF (IDMEF)
TARGET_LINK_LIBRARIES(collector commonUtils ipfixCollector ${XML_BLASTER_C_LIBRARIES} ${XML_BLASTER_CPP_LIBRARIES} ${XERCES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBXML2_LIBRARIES})
ELSE (IDMEF)
TARGET_LINK_LIBRARIES(collector commonUtils ipfixCollector  ${CMAKE_THREAD_LIBS_INIT} ${LIBXML2_LIBRARIES})
ENDIF (IDMEF)

IF (XML_BLASTER_FOUND AND IDMEF)
  INCLUDE_DIRECTORIES(${XML_BLASTER_INCLUDE_DIR})
ENDIF (XML_BLASTER_FOUND AND IDMEF)
