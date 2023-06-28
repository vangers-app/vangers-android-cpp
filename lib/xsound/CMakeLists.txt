include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/lib/xtool"
	"${PROJECT_SOURCE_DIR}/lib/xgraph"
)

SET(xsound_SRCS
	xsound.cpp
	avi.cpp
	ogg_stream.cpp
)


ADD_LIBRARY(xsound OBJECT ${xsound_SRCS})
