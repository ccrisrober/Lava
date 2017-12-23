########## GLSL2SPIRV ##########
##include( glsl2spirv )

##compileSPV( )

########## GLSL2SPIRV ##########

if( LAVA_WITH_EXAMPLES )

  include_directories(
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/routes.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/routes.h)

  set( LAVAVKINFO_HEADERS )
  set( LAVAVKINFO_SOURCES VkInfo.cpp )
  set( LAVAVKINFO_LINK_LIBRARIES lava )
  common_application( lavaVkInfo )

  set( LAVATRIANGLENOBUFFER_HEADERS )
  set( LAVATRIANGLENOBUFFER_SOURCES TriangleNoBuffer.cpp )
  set( LAVATRIANGLENOBUFFER_LINK_LIBRARIES lava )
  common_application( lavaTriangleNoBuffer )

  set( LAVADEMO_HEADERS )
  set( LAVADEMO_SOURCES Demo.cpp )
  set( LAVADEMO_LINK_LIBRARIES lava )
  common_application( lavaDemo )

endif( )