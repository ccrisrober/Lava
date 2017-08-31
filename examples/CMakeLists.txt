if( LAVA_WITH_EXAMPLES )

  include_directories(
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/routes.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/routes.h)

  set( LAVAINFO_HEADERS )
  set( LAVAINFO_SOURCES info.cpp )
  set( LAVAINFO_LINK_LIBRARIES
    lava
  )
  common_application( lavaInfo )

  set( LAVACLEARSCREENDEMO_HEADERS )
  set( LAVACLEARSCREENDEMO_SOURCES ClearScreenDemo.cpp )
  set( LAVACLEARSCREENDEMO_LINK_LIBRARIES
    lava
  )
  common_application( lavaClearScreenDemo )
  
endif( )