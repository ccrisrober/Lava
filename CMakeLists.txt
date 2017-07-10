project(vklava)
cmake_minimum_required(VERSION 3.1)
project( vklava VERSION 0.1.0 )
set( vklava_VERSION_ABI 1)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

include(GitExternal)

set(PROJECT_VERSION_ABI ${${PROJECT_NAME}_VERSION_ABI})

# Allow warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

include(Common)

set( COMMON_PROJECT_DOMAIN gmrv.es )
set( VKLAVA_DESCRIPTION "VKLAVA" )


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")


common_find_package( Vulkan REQUIRED SYSTEM )
common_find_package( GLFW3 REQUIRED SYSTEM )
common_find_package( GLM REQUIRED SYSTEM )

list( APPEND VKLAVA_DEPENDENT_LIBRARIES Vulkan GLFW3 GLM )

IF (NOT WIN32)
  link_libraries( -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread )
endif()

common_find_package_post( )

set( PROJECT_INCLUDE_NAME vklava )

add_subdirectory( vklava )

install( FILES ${PROJECT_BINARY_DIR}/include/vklava/defines.h
  DESTINATION include/vklava )
install( FILES ${PROJECT_BINARY_DIR}/include/vklava/defines${SYSTEM}.h
  DESTINATION include/vklava )

include( CPackConfig )
include( DoxygenRule )
