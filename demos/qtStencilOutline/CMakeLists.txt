set( POMPEIIQTSTENCILOUTLINE_HEADERS
	vulkanwindow.h
)
set( POMPEIIQTSTENCILOUTLINE_SOURCES
	main.cpp
)
set( POMPEIIQTSTENCILOUTLINE_LINK_LIBRARIES
	pompeii
	pompeiiUtils
	qtPompeii
)
common_application( pompeiiQtStencilOutline )