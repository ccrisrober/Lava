set( LAVAQTSTENCILOUTLINE_HEADERS
	vulkanWindow.h
)
set( LAVAQTSTENCILOUTLINE_SOURCES
	main.cpp
)
set( LAVAQTSTENCILOUTLINE_LINK_LIBRARIES
	lava
	lavaUtils
	qtLava
)
common_application( lavaQtStencilOutline )