set( POMPEIIGLFWDEFERREDSUBPASS_HEADERS
)
set( POMPEIIGLFWDEFERREDSUBPASS_SOURCES
	main.cpp
)
set( POMPEIIGLFWDEFERREDSUBPASS_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFWDeferredSubpass )