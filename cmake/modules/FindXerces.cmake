# Based on http://public.kitware.com/pipermail/cmake/2003-May/003820.html
# changes by Lothar Braun <mail@lobraun.de>
# and Raimondas Sasnauskas <sasnausk@informatik.uni-tuebingen.de>

#
# Find the native Xerces includes and library
#
# XERCES_INCLUDE_DIR - where to find dom/dom.hpp, etc.
# XERCES_LIBRARIES   - List of fully qualified libraries to link against
# when using Xerces.
# XERCES_FOUND       - Do not attempt to use Xerces if "no" or undefined.

MESSAGE(STATUS "Searching for libxerces")

FIND_PATH(XERCES_INCLUDE_DIR 
  NAMES dom/dom.hpp dom/DOM.hpp
  PATHS
  /usr/local/include
  /usr/include
  /usr/include/xercesc
  /usr/local/include/xercesc
)

# Make sure that the Xerces include path has been set
# So the XERCES_LIBRARY does not appear the first time
IF(XERCES_INCLUDE_DIR)
  FIND_LIBRARY(XERCES_LIBRARY
    NAMES
      xerces-c_2
      xerces-c_2D
      xerces-c
    PATHS
      /usr/local/lib
      /usr/lib
      ${XERCES_INCLUDE_DIR}/../lib
  )
ENDIF(XERCES_INCLUDE_DIR)

IF(XERCES_INCLUDE_DIR)
  IF(XERCES_LIBRARY)
    SET( XERCES_LIBRARIES ${XERCES_LIBRARY} )
    SET( XERCES_FOUND "YES" )
  ENDIF(XERCES_LIBRARY)
ENDIF(XERCES_INCLUDE_DIR)
