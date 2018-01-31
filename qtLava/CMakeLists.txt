#source_group(\\ FILES CMakeLists.txt)

set( QTLAVA_PUBLIC_HEADERS
	DefaultFramebuffer.h
	QVulkanWindow.h
)

set( QTLAVA_SOURCES
	DefaultFramebuffer.cpp
  	QVulkanWindow.cpp
)

set( QTLAVA_HEADERS
)

set( QTLAVA_MOC_HEADERS
)

set( QTLAVA_LINK_LIBRARIES
	lava
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
)
set(QTLAVA_INCLUDE_NAME qtLava )
set(QTLAVA_NAMESPACE qtLava )
common_library( qtLava )
