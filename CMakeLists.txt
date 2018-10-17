cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project( pompeii VERSION 2.1.0 )
set( pompeii_VERSION_ABI 3)

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
set( POMPEII_DESCRIPTION "POMPEII" )

include(Common)

# Allow warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")


option( POMPEII_WITH_DEVICE_BUILDERS "enabling/disabling custom device builder" ON )
option( POMPEII_WITH_EXAMPLES "POMPEII examples" ON)
#option( POMPEIIENGINE_WITH_COMPONENTS "POMPEII with components" ON )

#if ( POMPEIIENGINE_WITH_COMPONENTS )
#  list(APPEND COMMON_FIND_PACKAGE_DEFINES POMPEIIENGINE_HASCOMPONENTS)
#endif( )

if ( POMPEII_WITH_DEVICE_BUILDERS )
  list(APPEND COMMON_FIND_PACKAGE_DEFINES POMPEII_DEVICE_BUILDERS )
endif( )

set( STBI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/stbi" )

common_find_package( Vulkan REQUIRED SYSTEM )
common_find_package( STBI REQUIRED SYSTEM )

common_find_package( GLM SYSTEM )
common_find_package( ASSIMP SYSTEM )
common_find_package( GLFW3 SYSTEM 3.2.1 )
common_find_package( Qt5Core SYSTEM 5.10 )
common_find_package( Qt5Widgets SYSTEM 5.10 )

common_find_package( Boost COMPONENTS unit_test_framework )


list( APPEND POMPEII_DEPENDENT_LIBRARIES Vulkan STBI )

IF (NOT WIN32)
  link_libraries( -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread )
endif()

common_find_package_post( )


add_subdirectory( pompeii )

#add glslang
option( ENABLE_GLSLANG_BINARIES "Enable GLSLang Binaries" OFF )
option( BUILD_TESTING OFF )
option( ENABLE_HLSL OFF)
add_subdirectory( vendor/glslang )

#if( GLFW3_FOUND )
#  add_subdirectory( glfwPompeii )
#endif( )

#if( QT5CORE_FOUND )
#  add_subdirectory( qtPompeii )
#endif( )

add_subdirectory( pompeiiUtils )
#add_subdirectory( pompeiiEngine )

add_subdirectory( demos )

#if(${Boost_FOUND})
#  add_subdirectory(tests)
#endif()

configure_file(
  CMake/glsl2spv.cmake
  ${PROJECT_BINARY_DIR}/${CMAKE_MODULE_INSTALL_PATH}/glsl2spv.cmake
  COPYONLY
  )
install( FILES CMake/glsl2spv.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT dev )

set(DOXYGEN_MAINPAGE_MD ${PROJECT_SOURCE_DIR}/README.md)
set(DOXYGEN_EXTRA_INPUT ${DOXYGEN_MAINPAGE_MD})
# TODO: CRISTIAN ARREGLAR include(CPackConfig)
include(DoxygenRule)
