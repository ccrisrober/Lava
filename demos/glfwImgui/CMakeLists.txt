set( POMPEIIGLFWIMGUI_HEADERS
)
set( POMPEIIGLFWIMGUI_SOURCES
	main.cpp
)
set( POMPEIIGLFWIMGUI_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWImgui )