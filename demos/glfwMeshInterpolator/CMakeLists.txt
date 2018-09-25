set( POMPEIIGLFWMESHINTERPOLATOR_HEADERS
)
set( POMPEIIGLFWMESHINTERPOLATOR_SOURCES
	main.cpp
)
set( POMPEIIGLFWMESHINTERPOLATOR_LINK_LIBRARIES
	pompeii
	pompeiiUtils
	glfwPompeii
)
common_application( pompeiiGLFWMeshInterpolator )