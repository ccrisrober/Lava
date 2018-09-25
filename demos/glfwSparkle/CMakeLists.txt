set( POMPEIIGLFWSPARKLE_HEADERS
)
set( POMPEIIGLFWSPARKLE_SOURCES
	main.cpp
)
set( POMPEIIGLFWSPARKLE_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWSparkle )