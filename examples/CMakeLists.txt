if( LAVA_WITH_EXAMPLES )

  include_directories(
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/routes.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/routes.h)

  set( LAVAWINDOW_HEADERS )
  set( LAVAWINDOW_SOURCES window.cpp )
  set( LAVAWINDOW_LINK_LIBRARIES
    ##${GLFW3_LIBRARIES}
    ##${GLM_LIBRARIES}
    lava
  )
  common_application( lavaWindow )

  set( LAVAINFO_HEADERS )
  set( LAVAINFO_SOURCES info.cpp )
  set( LAVAINFO_LINK_LIBRARIES
    ##${GLFW3_LIBRARIES}
    ##${GLM_LIBRARIES}
    lava
  )
  common_application( lavaInfo )
  
endif( )