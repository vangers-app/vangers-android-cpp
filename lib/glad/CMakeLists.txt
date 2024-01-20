SET(glad_SRCS
	src/glad.c
	include/glad/glad.h
)

if (ANDROID)
	ADD_LIBRARY(glad OBJECT ${glad_SRCS})
else()
	ADD_LIBRARY(glad STATIC ${glad_SRCS})
endif()

include_directories(
	"${PROJECT_SOURCE_DIR}/lib/glad/include"
)

target_include_directories( 
	glad 
	INTERFACE "${PROJECT_SOURCE_DIR}/lib/glad/include"
)

