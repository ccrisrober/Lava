set( Name vklava )
set( NAME VKLAVA )
set( namespace vklava )
set( NAMESPACE VKLAVA )
set( INCLUDE_NAME vklava )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/api.h
  ${PROJECT_BINARY_DIR}/include/vklava/api.h @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.h
  ${PROJECT_BINARY_DIR}/include/vklava/version.h @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/CMake/common/cpp/version.cpp
  ${PROJECT_BINARY_DIR}/src/vklava/version.cpp @ONLY )

set( VKLAVA_SOURCES
  vklava.cpp
)

set( VKLAVA_HEADERS
)

set( VKLAVA_MOC_HEADERS
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set( VKLAVA_LINK_LIBRARIES
    ${VULKAN_LIBRARY}
    glfw
)

if ( MSVC )
  add_definitions( -DVKLAVA_STATIC )
endif( )

common_application( vkLava )
