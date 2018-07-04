set( LAVAGLFWSILHOUETTE_HEADERS
)
set( LAVAGLFWSILHOUETTE_SOURCES
	main.cpp
)
set( LAVAGLFWSILHOUETTE_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWSilhouette )