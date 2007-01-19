PROJECT(COLLECTOR)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SUBDIRS(concentrator commonutils collector detectionmodules)

FIND_PACKAGE(Threads)
FIND_PACKAGE(Doxygen)

#################################### what about doxygen? ######################################

IF (DOXYGEN)
  MESSAGE(STATUS "Found doxygen")
ELSE (DOXYGEN)
  MESSAGE(STATUS "Could not find doxygen")
ENDIF (DOXYGEN)


#################################### check for pthreads ########################################

IF (CMAKE_USE_PTHREADS_INIT)
  MESSAGE(STATUS "Found pthread library")
ELSE (CMAKE_USE_PTHREADS_INIT)
  MESSAGE(FATAL_ERROR "Could not find a suitable pthread library")
ENDIF (CMAKE_USE_PTHREADS_INIT)


#################################### Locate libxml2 ############################################

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/modules/FindLibXML2.cmake)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${LIBXML2_INCLUDE_DIR})
LINK_DIRECTORIES(${CMAKE_BUILD_DIR}/concentrator ${CMAKE_BUILD_DIR}/commonutils)

#################################### Locate gsl ################################################

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/modules/FindGSL.cmake)

#################################### Check for libXerces #######################################

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/modules/FindXerces.cmake)

#################################### Look for xmlBlaster #######################################

OPTION(IDMEF "Enable/Disable IDMEF-Support. Requires xmlBlaster if enabled." ON)

IF (IDMEF)
  message(STATUS "Looking for xmlBLaster...")
  INCLUDE(${CMAKE_SOURCE_DIR}/cmake/modules/FindXmlBlaster.cmake)
  IF (NOT XERCES_FOUND)
    message(FATAL_ERROR "Xerces not installed! ${XERCES_FOUND}")
  ENDIF (NOT XERCES_FOUND)
  message(STATUS "Applying defines...")
  ADD_DEFINITIONS(-DIDMEF_SUPPORT_ENABLED)
ENDIF (IDMEF)

################################### Locate Glib2 ###############################################

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/modules/FindGlib.cmake)

ADD_DEFINITIONS(-g -pg)
