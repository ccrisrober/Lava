set( LAVAGLFWSPARKLE_HEADERS
)
set( LAVAGLFWSPARKLE_SOURCES
	main.cpp
)
set( LAVAGLFWSPARKLE_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWSparkle )