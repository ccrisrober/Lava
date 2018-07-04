set( LAVAGLFWINPUTPROCESS_HEADERS
)
set( LAVAGLFWINPUTPROCESS_SOURCES
	main.cpp
)
set( LAVAGLFWINPUTPROCESS_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWInputProcess )