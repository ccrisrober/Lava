set( POMPEIIGLFWMESHNORMALS_HEADERS
)
set( POMPEIIGLFWMESHNORMALS_SOURCES
	main.cpp
)
set( POMPEIIGLFWMESHNORMALS_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWMeshNormals )