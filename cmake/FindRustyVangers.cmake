IF(MINGW)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
ENDIF()

FIND_LIBRARY(RUSTYVANGERS_LIBRARY
	NAMES
	    rusty_vangers
	PATHS
	    "${RUSTYVANGERS_ROOT}/target/release"
)
IF(RUSTYVANGERS_LIBRARY)
    SET(RUSTYVANGERS_FOUND TRUE)
ENDIF(RUSTYVANGERS_LIBRARY)

IF(RUSTYVANGERS_FOUND)
    IF(NOT RUSTYVANGERS_FIND_QUIETLY)
	    MESSAGE(STATUS "Found rusty_vangers library: ${RUSTYVANGERS_LIBRARY}")
	ENDIF(NOT RUSTYVANGERS_FIND_QUIETLY)
ELSE(RUSTYVANGERS_FOUND)
    IF(RUSTYVANGERS_FIND_REQUIRED)
	    MESSAGE(FATAL_ERROR "Could not find rusty_vangers library")
	ENDIF(RUSTYVANGERS_FIND_REQUIRED)
ENDIF(RUSTYVANGERS_FOUND)
