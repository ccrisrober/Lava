set( LAVAQTSAMPLEUNIFORM_HEADERS
	vulkanWindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTSAMPLEUNIFORM_SOURCES
	vulkanWindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( LAVAQTSAMPLEUNIFORM_LINK_LIBRARIES
	lava
	qtLava
)
common_application( lavaQtSampleUniform )