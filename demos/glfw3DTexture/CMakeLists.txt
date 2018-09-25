set( POMPEIIGLFW3DTEXTURE_HEADERS
)
set( POMPEIIGLFW3DTEXTURE_SOURCES
	main.cpp
)
set( POMPEIIGLFW3DTEXTURE_LINK_LIBRARIES
	pompeii
	glfwPompeii
	pompeiiUtils
)
common_application( pompeiiGLFW3DTexture )