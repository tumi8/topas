ADD_EXECUTABLE(collector collectorconfobj.cpp collector.cpp collector_main.cpp detectmod.cpp detectmodexporter.cpp manager.cpp modulecontainer.cpp recorder.cpp)
TARGET_LINK_LIBRARIES(collector commonUtils ipfixCollector ${LIBXML2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
