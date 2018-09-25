set( POMPEIIGLFWINPUTPROCESS_HEADERS
)
set( POMPEIIGLFWINPUTPROCESS_SOURCES
	main.cpp
)
set( POMPEIIGLFWINPUTPROCESS_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWInputProcess )