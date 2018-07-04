set( LAVAGLFWTHREADPOOL_HEADERS
)
set( LAVAGLFWTHREADPOOL_SOURCES
	main.cpp
)
set( LAVAGLFWTHREADPOOL_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWThreadPool )