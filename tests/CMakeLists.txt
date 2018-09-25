include_directories( ${CMAKE_BINARY_DIR}/include )
include_directories( ${PROJECT_SOURCE_DIR} )

if(${Boost_FOUND})
	set(TEST_LIBRARIES
		${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	    pompeii
	    pompeiiUtils
	)
	include(CommonCTest)
endif()