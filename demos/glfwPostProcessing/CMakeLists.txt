set( POMPEIIGLFWPOSTPROCESSING_HEADERS
)
set( POMPEIIGLFWPOSTPROCESSING_SOURCES
	main.cpp
)
set( POMPEIIGLFWPOSTPROCESSING_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWPostProcessing )