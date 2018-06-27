set( LAVAGLFWRENDER_HEADERS
)
set( LAVAGLFWRENDER_SOURCES
	main.cpp
)
set( LAVAGLFWRENDER_LINK_LIBRARIES
	lava
	lavaRenderer
)
common_application( lavaGLFWRender )