set( LAVAGLFWRENDER_HEADERS
)
set( LAVAGLFWRENDER_SOURCES
	main.cpp
)
set( LAVAGLFWRENDER_LINK_LIBRARIES
	lava
	glfwLava
)
common_application( lavaGLFWRender )