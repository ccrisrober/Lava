set( POMPEIIQTRENDER_HEADERS
	mainwindow.h
)
set( POMPEIIQTRENDER_SOURCES
	main.cpp
	mainwindow.cpp
)
set( POMPEIIQTRENDER_LINK_LIBRARIES
	pompeii
	qtPompeii
)
common_application( pompeiiQtRender )