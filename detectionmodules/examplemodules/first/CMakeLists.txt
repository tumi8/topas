ADD_EXECUTABLE(examplemodule main.cpp examplemodule.cpp exampledatastorage.cpp)
TARGET_LINK_LIBRARIES(examplemodule detectionBase commonUtils ipfixCollector ${LIBXML2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

OPTION(OFFLINE "Use detection module offline (OFFLINE_ENABLED)." OFF)
IF (OFFLINE)
  ADD_DEFINITIONS(-DOFFLINE_ENABLED)
ELSE (OFFLINE)
  REMOVE_DEFINITIONS(-DOFFLINE_ENABLED)
ENDIF (OFFLINE)

