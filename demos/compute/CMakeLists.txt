set( POMPEIICOMPUTECOPY_HEADERS
)
set( POMPEIICOMPUTECOPY_SOURCES
	ComputeCopy.cpp
)
set( POMPEIICOMPUTECOPY_LINK_LIBRARIES
	pompeii
	pompeiiUtils
)
common_application( pompeiiComputeCopy )

# ------------------------------------
set( POMPEIICOMPUTESUM_HEADERS
)
set( POMPEIICOMPUTESUM_SOURCES
	ComputeSum.cpp
)
set( POMPEIICOMPUTESUM_LINK_LIBRARIES
	pompeii
	pompeiiUtils
)
common_application( pompeiiComputeSum )