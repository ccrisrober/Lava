cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project( lava VERSION 0.1.0 )
set( lava_VERSION_ABI 1)

# Disable in source building
if( "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" )
    message( FATAL "no in source building allowed." )
endif()

list(APPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/CMake
    ${CMAKE_SOURCE_DIR}/CMake/common)

# Default build type is Debug
if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Debug )
endif( NOT CMAKE_BUILD_TYPE )

if ( CMAKE_BUILD_TYPE MATCHES "Debug" )
  add_definitions( -DDEBUG )
endif( )

include(GitExternal)

set( COMMON_PROJECT_DOMAIN maldicion069 )
set( LAVA_DESCRIPTION "LAVA" )

include(Common)
# Allow warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")

option( LAVA_WITH_EXAMPLES "LAVA examples" ON)


common_find_package( Vulkan REQUIRED SYSTEM )
common_find_package( GLFW3 REQUIRED SYSTEM )
common_find_package( GLM REQUIRED SYSTEM )
common_find_package( STBI REQUIRED SYSTEM )
common_find_package( ASSIMP REQUIRED SYSTEM )
#common_find_package( GLSLANG REQUIRED )

list( APPEND LAVA_DEPENDENT_LIBRARIES Vulkan GLFW3 GLM STBI ASSIMP 
#GLSLANG
)

# TODO( FIXED GLFW LINUX )
IF (NOT WIN32)
  link_libraries( -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread )
endif()

common_find_package_post( )

add_subdirectory( lava )
#add_subdirectory( lava_extras )
add_subdirectory( examples )
#add_subdirectory( glslang )


set(DOXYGEN_MAINPAGE_MD ${PROJECT_SOURCE_DIR}/README.md)
set(DOXYGEN_EXTRA_INPUT ${DOXYGEN_MAINPAGE_MD})
include(CPackConfig)
include(DoxygenRule)
