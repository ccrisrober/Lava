if( LAVA_WITH_EXAMPLES )
  include( glsl2spv )

  glsl2spv( )
  
  include_directories(
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/routes.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/routes.h)

if( QT5CORE_FOUND )
  set( LAVAQTRENDER_HEADERS )
  set( LAVAQTRENDER_SOURCES QtRender.cpp )
  set( LAVAQTRENDER_LINK_LIBRARIES lava qtLava )
  common_application( lavaQtRender )
endif( )

endif( )