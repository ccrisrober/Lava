set( POMPEIIRENDERHEADLESS_HEADERS
)
set( POMPEIIRENDERHEADLESS_SOURCES
	RenderHeadless.cpp
)
set( POMPEIIRENDERHEADLESS_LINK_LIBRARIES
	pompeii
)
common_application( pompeiiRenderHeadless )

set( POMPEIIFOODEMO_HEADERS
)
set( POMPEIIFOODEMO_SOURCES
	FooDemo.cpp
)
set( POMPEIIFOODEMO_LINK_LIBRARIES
	pompeii
	pompeiiUtils
)
common_application( pompeiiFooDemo )