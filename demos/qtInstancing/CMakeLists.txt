set( LAVAQTINSTANCING_HEADERS
	vulkanWindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTINSTANCING_SOURCES
	vulkanWindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( LAVAQTINSTANCING_LINK_LIBRARIES
	lava
	qtLava
)
common_application( lavaQtInstancing )