set( POMPEIIGLFWSHADOWMAPPING_HEADERS
)
set( POMPEIIGLFWSHADOWMAPPING_SOURCES
	main.cpp
)
set( POMPEIIGLFWSHADOWMAPPING_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWShadowMapping )