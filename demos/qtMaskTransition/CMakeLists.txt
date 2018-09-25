set( POMPEIIQTMASKTRANSITION_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( POMPEIIQTMASKTRANSITION_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( POMPEIIQTMASKTRANSITION_LINK_LIBRARIES
	pompeii
	qtPompeii
)
common_application( pompeiiQtMaskTransition )