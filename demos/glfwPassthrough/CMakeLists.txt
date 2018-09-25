set( POMPEIIGLFWPASSTHROUGH_HEADERS
)
set( POMPEIIGLFWPASSTHROUGH_SOURCES
	main.cpp
)
set( POMPEIIGLFWPASSTHROUGH_LINK_LIBRARIES
	pompeii
	glfwPompeii
)
common_application( pompeiiGLFWPassthrough )