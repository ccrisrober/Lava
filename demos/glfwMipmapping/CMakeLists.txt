set( LAVAGLFWMIPMAPPING_HEADERS
)
set( LAVAGLFWMIPMAPPING_SOURCES
	main.cpp
)
set( LAVAGLFWMIPMAPPING_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWMipmapping )