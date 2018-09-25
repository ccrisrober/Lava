set( POMPEIIQTHEIGHTMAPTESSELATION_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( POMPEIIQTHEIGHTMAPTESSELATION_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( POMPEIIQTHEIGHTMAPTESSELATION_LINK_LIBRARIES
	pompeii
	qtPompeii
)
common_application( pompeiiQtHeightmapTesselation )