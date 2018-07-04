set( LAVAQTSAMPLEUNIFORM_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( LAVAQTSAMPLEUNIFORM_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( LAVAQTSAMPLEUNIFORM_LINK_LIBRARIES
	lava
	qtLava
)
common_application( lavaQtSampleUniform )