set( LAVAQTINSTANCING_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTINSTANCING_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( LAVAQTINSTANCING_LINK_LIBRARIES
	lava
	qtLava
)
common_application( lavaQtInstancing )