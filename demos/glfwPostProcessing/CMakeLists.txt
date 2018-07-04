set( LAVAGLFWPOSTPROCESSING_HEADERS
)
set( LAVAGLFWPOSTPROCESSING_SOURCES
	main.cpp
)
set( LAVAGLFWPOSTPROCESSING_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWPostProcessing )