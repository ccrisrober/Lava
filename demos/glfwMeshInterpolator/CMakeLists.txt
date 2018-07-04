set( LAVAGLFWMESHINTERPOLATOR_HEADERS
)
set( LAVAGLFWMESHINTERPOLATOR_SOURCES
	main.cpp
)
set( LAVAGLFWMESHINTERPOLATOR_LINK_LIBRARIES
	lava
	lavaUtils
	glfwLava
)
common_application( lavaGLFWMeshInterpolator )