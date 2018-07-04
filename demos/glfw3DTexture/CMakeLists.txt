set( LAVAGLFW3DTEXTURE_HEADERS
)
set( LAVAGLFW3DTEXTURE_SOURCES
	main.cpp
)
set( LAVAGLFW3DTEXTURE_LINK_LIBRARIES
	lava
	glfwLava
	lavaUtils
)
common_application( lavaGLFW3DTexture )