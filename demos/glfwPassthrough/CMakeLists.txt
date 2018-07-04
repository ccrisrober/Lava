set( LAVAGLFWPASSTHROUGH_HEADERS
)
set( LAVAGLFWPASSTHROUGH_SOURCES
	main.cpp
)
set( LAVAGLFWPASSTHROUGH_LINK_LIBRARIES
	lava
	glfwLava
)
common_application( lavaGLFWPassthrough )