include_directories(
	"${PROJECT_SOURCE_DIR}/lib/xtool"
)

SET(xgraph_SRCS
	xfont.cpp
	xgraph.cpp
	xbmp.cpp
	xside.cpp
)

ADD_LIBRARY(xgraph OBJECT ${xgraph_SRCS})

