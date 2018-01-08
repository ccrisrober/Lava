cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project( lava VERSION 0.1.2 )
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


option( LAVA_WITH_DEVICE_BUILDERS "enabling/disabling custom device builder" ON )
option( LAVA_WITH_EXAMPLES "LAVA examples" ON)

if ( LAVA_WITH_DEVICE_BUILDERS )
  list(APPEND COMMON_FIND_PACKAGE_DEFINES LAVA_DEVICE_BUILDERS )
endif( )


set(STBI_INCLUDE_DIR ../stbi)

common_find_package( Vulkan REQUIRED SYSTEM )
common_find_package( GLFW3 REQUIRED SYSTEM )
common_find_package( GLM REQUIRED SYSTEM )
common_find_package( STBI REQUIRED SYSTEM )
common_find_package( ASSIMP REQUIRED SYSTEM )
#common_find_package( GLSLANG REQUIRED )

list( APPEND LAVA_DEPENDENT_LIBRARIES Vulkan GLFW3 GLM STBI ASSIMP 
#GLSLANG
)

IF (NOT WIN32)
  link_libraries( -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread )
endif()

common_find_package_post( )

add_subdirectory( lava )
add_subdirectory( examples )


file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.frag"
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.comp"
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.geom"
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.tesc"
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.tese"
)
if ( WIN32 )
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else( )
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslangValidator")
endif( )
foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME_WE)
  get_filename_component(FILE_EXT_AUX ${GLSL} EXT)
  STRING(REPLACE "." "" FILE_EXT ${FILE_EXT_AUX})
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}_${FILE_EXT}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
  Shaders 
  DEPENDS ${SPIRV_BINARY_FILES}
)

set(DOXYGEN_MAINPAGE_MD ${PROJECT_SOURCE_DIR}/README.md)
set(DOXYGEN_EXTRA_INPUT ${DOXYGEN_MAINPAGE_MD})
include(CPackConfig)
include(DoxygenRule)
