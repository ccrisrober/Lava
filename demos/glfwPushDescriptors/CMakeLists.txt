set( POMPEIIGLFWPUSHDESCRIPTORS_HEADERS
)
set( POMPEIIGLFWPUSHDESCRIPTORS_SOURCES
	main.cpp
)
set( POMPEIIGLFWPUSHDESCRIPTORS_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWPushDescriptors )