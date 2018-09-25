set( POMPEIIQTSAMPLEUNIFORM_HEADERS
	vulkanwindow.h
	renderer.h
	mainwindow.h
)
set( POMPEIIQTSAMPLEUNIFORM_SOURCES
	vulkanwindow.cpp
	renderer.cpp
	mainwindow.cpp
	main.cpp
)
set( POMPEIIQTSAMPLEUNIFORM_LINK_LIBRARIES
	pompeii
	qtPompeii
)
common_application( pompeiiQtSampleUniform )