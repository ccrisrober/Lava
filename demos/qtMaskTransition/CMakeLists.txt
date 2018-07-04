set( LAVAQTMASKTRANSITION_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTMASKTRANSITION_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( LAVAQTMASKTRANSITION_LINK_LIBRARIES
	lava
	qtLava
)
common_application( lavaQtMaskTransition )