set( LAVAQTHEIGHTMAPTESSELATION_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTHEIGHTMAPTESSELATION_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( LAVAQTHEIGHTMAPTESSELATION_LINK_LIBRARIES
	lava
	qtLava
)
common_application( lavaQtHeightmapTesselation )