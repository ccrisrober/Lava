#source_group(\\ FILES CMakeLists.txt)

set( QTPOMPEII_PUBLIC_HEADERS
	QVulkanWindow.h
)

set( QTPOMPEII_SOURCES
	QVulkanWindow.cpp
)

set( QTPOMPEII_HEADERS
)

set( QTPOMPEII_MOC_HEADERS
)

set( QTPOMPEII_LINK_LIBRARIES
	pompeii
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
)
set(QTPOMPEII_INCLUDE_NAME qtLava )
set(QTPOMPEII_NAMESPACE qtLava )
common_library( qtLava )
