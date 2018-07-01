set( LAVAGLFWMESHNORMALS_HEADERS
)
set( LAVAGLFWMESHNORMALS_SOURCES
	main.cpp
)
set( LAVAGLFWMESHNORMALS_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWMeshNormals )