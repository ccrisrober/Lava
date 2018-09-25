set( POMPEIIGLFWMULTISETDESCRIPTOR_HEADERS
)
set( POMPEIIGLFWMULTISETDESCRIPTOR_SOURCES
	main.cpp
)
set( POMPEIIGLFWMULTISETDESCRIPTOR_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWMultiSetDescriptor )