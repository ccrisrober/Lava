set( POMPEIIQTINSTANCING_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( POMPEIIQTINSTANCING_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( POMPEIIQTINSTANCING_LINK_LIBRARIES
	pompeii
	qtPompeii
)
common_application( pompeiiQtInstancing )