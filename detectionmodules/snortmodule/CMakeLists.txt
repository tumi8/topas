ADD_EXECUTABLE(snortmodule main.cpp pcapwriter.cpp pcappacket.cpp
snortmodule.cpp snortstore.cpp)
TARGET_LINK_LIBRARIES(snortmodule detectionBase commonUtils ipfixCollector
${LIBXML2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${GLIB2_LIBRARIES})
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIR} /usr/lib/glib-2.0/include/)
