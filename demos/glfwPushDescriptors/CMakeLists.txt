set( LAVAGLFWPUSHDESCRIPTORS_HEADERS
)
set( LAVAGLFWPUSHDESCRIPTORS_SOURCES
	main.cpp
)
set( LAVAGLFWPUSHDESCRIPTORS_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWPushDescriptors )