cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project( lava VERSION 2.1.0 )
set( lava_VERSION_ABI 3)

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
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")


option( LAVA_WITH_DEVICE_BUILDERS "enabling/disabling custom device builder" ON )
option( LAVA_WITH_EXAMPLES "LAVA examples" ON)
option( LAVAENGINE_WITH_COMPONENTS "LAVA with components" ON )

if ( LAVAENGINE_WITH_COMPONENTS )
  list(APPEND COMMON_FIND_PACKAGE_DEFINES LAVAENGINE_HASCOMPONENTS)
endif( )

if ( LAVA_WITH_DEVICE_BUILDERS )
  list(APPEND COMMON_FIND_PACKAGE_DEFINES LAVA_DEVICE_BUILDERS )
endif( )


set(STBI_INCLUDE_DIR ../stbi)

common_find_package( Vulkan REQUIRED SYSTEM )
common_find_package( STBI REQUIRED SYSTEM )

common_find_package( GLM SYSTEM )
common_find_package( ASSIMP SYSTEM )
common_find_package( GLFW3 SYSTEM )
common_find_package( Qt5Core SYSTEM 5.10 )
common_find_package( Qt5Widgets SYSTEM 5.10 )

common_find_package( Boost COMPONENTS unit_test_framework )


list( APPEND LAVA_DEPENDENT_LIBRARIES Vulkan STBI )

IF (NOT WIN32)
  link_libraries( -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread )
endif()

common_find_package_post( )

#option(ENABLE_GLSLANG_BINARIES "Enable GLSLang Binaries" OFF)
#add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glslang")
#SET( ENABLE_HLSL OFF CACHE BOOL "Enables HLSL input support" )

add_subdirectory( lava )

if( GLFW3_FOUND )
  add_subdirectory( glfwLava )
endif( )

if( QT5CORE_FOUND )
  add_subdirectory( qtLava )
endif( )

add_subdirectory( lavaUtils )
#add_subdirectory( lavaEngine )

add_subdirectory( demos )

if(${Boost_FOUND})
  add_subdirectory(tests)
endif()

configure_file(
  CMake/glsl2spv.cmake
  ${PROJECT_BINARY_DIR}/${CMAKE_MODULE_INSTALL_PATH}/glsl2spv.cmake
  COPYONLY
  )
install( FILES CMake/glsl2spv.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT dev )

set(DOXYGEN_MAINPAGE_MD ${PROJECT_SOURCE_DIR}/README.md)
set(DOXYGEN_EXTRA_INPUT ${DOXYGEN_MAINPAGE_MD})
include(CPackConfig)
include(DoxygenRule)
