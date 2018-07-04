set( LAVAGLFWIMGUI_HEADERS
)
set( LAVAGLFWIMGUI_SOURCES
	main.cpp
)
set( LAVAGLFWIMGUI_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFWImgui )