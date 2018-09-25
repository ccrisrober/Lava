set( POMPEIIQTSKYBOX_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( POMPEIIQTSKYBOX_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( POMPEIIQTSKYBOX_LINK_LIBRARIES
	pompeii
	pompeiiUtils
	qtPompeii
)
common_application( pompeiiQtSkybox )