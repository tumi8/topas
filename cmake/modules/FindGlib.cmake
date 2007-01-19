FIND_PATH(
  GLIB2_INCLUDE_DIR
  glib.h
  PATHS /usr/include/glib-2.0 /usr/local/include/glib-2.0
  /usr/include /usr/local/include
  /opt/gnome/include /opt/gnome/include/glib-2.0
  )


FIND_LIBRARY (
  GLIB2_LIBRARIES
  NAMES  libglib-2.0 glib2 glib-2.0
  PATHS /usr/lib /usr/local/lib /opt/gnome/lib/
  PATH_SUFFIXES glib glib2
  )

IF (GLIB2_INCLUDE_DIR AND GLIB2_LIBRARIES)
  SET(GLIB2_FOUND TRUE)
ENDIF (GLIB2_INCLUDE_DIR AND GLIB2_LIBRARIES)

IF (GLIB2_FOUND)
  MESSAGE(STATUS "Found glib2")
ELSE (GLIB2_FOUND)
  MESSAGE(FATAL_ERROR "Could not find glib2")
ENDIF (GLIB2_FOUND)
