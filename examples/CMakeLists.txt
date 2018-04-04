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

  if( GLFW3_FOUND )
    set( LAVACLEARSCREEN_HEADERS )
    set( LAVACLEARSCREEN_SOURCES ClearScreen.cpp )
    set( LAVACLEARSCREEN_LINK_LIBRARIES lava lavaRenderer )
    common_application( lavaClearScreen )
  endif( )
endif( )