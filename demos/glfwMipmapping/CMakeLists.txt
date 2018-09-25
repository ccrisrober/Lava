set( POMPEIIGLFWMIPMAPPING_HEADERS
)
set( POMPEIIGLFWMIPMAPPING_SOURCES
	main.cpp
)
set( POMPEIIGLFWMIPMAPPING_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWMipmapping )