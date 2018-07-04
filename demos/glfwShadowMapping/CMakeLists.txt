set( LAVAGLFWSHADOWMAPPING_HEADERS
)
set( LAVAGLFWSHADOWMAPPING_SOURCES
	main.cpp
)
set( LAVAGLFWSHADOWMAPPING_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWShadowMapping )