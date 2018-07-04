set( LAVACOMPUTECOPY_HEADERS
)
set( LAVACOMPUTECOPY_SOURCES
	ComputeCopy.cpp
)
set( LAVACOMPUTECOPY_LINK_LIBRARIES
	lava
	lavaUtils
)
common_application( lavaComputeCopy )

# ------------------------------------
set( LAVACOMPUTESUM_HEADERS
)
set( LAVACOMPUTESUM_SOURCES
	ComputeSum.cpp
)
set( LAVACOMPUTESUM_LINK_LIBRARIES
	lava
	lavaUtils
)
common_application( lavaComputeSum )