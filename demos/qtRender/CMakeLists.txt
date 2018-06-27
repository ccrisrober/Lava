set( LAVAQTRENDER_HEADERS
	mainwindow.h
)
set( LAVAQTRENDER_SOURCES
	main.cpp
	mainwindow.cpp
)
set( LAVAQTRENDER_LINK_LIBRARIES
	lava
	qtLava
)
common_application( lavaQtRender )