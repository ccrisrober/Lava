set( LAVAQTSKYBOX_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTSKYBOX_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( LAVAQTSKYBOX_LINK_LIBRARIES
	lava
	lavaUtils
	qtLava
)
common_application( lavaQtSkybox )