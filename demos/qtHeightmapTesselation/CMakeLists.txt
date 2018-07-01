set( LAVAQTHEIGHTMAPTESSELATION_HEADERS
	vulkanWindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTHEIGHTMAPTESSELATION_SOURCES
	vulkanWindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( LAVAQTHEIGHTMAPTESSELATION_LINK_LIBRARIES
	lava
	qtLava
)
common_application( lavaQtHeightmapTesselation )