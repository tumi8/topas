ADD_EXECUTABLE(examplemodule main.cpp examplemodule.cpp exampledatastorage.cpp)
TARGET_LINK_LIBRARIES(examplemodule detectionBase commonUtils ipfixCollector ${LIBXML2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
