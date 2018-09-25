set( POMPEIIGLFWTHREADPOOL_HEADERS
)
set( POMPEIIGLFWTHREADPOOL_SOURCES
	main.cpp
)
set( POMPEIIGLFWTHREADPOOL_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWThreadPool )