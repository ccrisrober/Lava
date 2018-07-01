set( LAVAQTSKYBOX_HEADERS
	vulkanWindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTSKYBOX_SOURCES
	vulkanWindow.cpp
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