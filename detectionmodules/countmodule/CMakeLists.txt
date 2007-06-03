ADD_EXECUTABLE(countmodule main.cpp countmodule.cpp countstore.cpp bloomfilter.cpp agebloomfilter.cpp)
TARGET_LINK_LIBRARIES(countmodule detectionBase commonUtils ipfixCollector ${LIBXML2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${GSL_LIBRARIES})
