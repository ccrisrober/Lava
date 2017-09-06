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

  set( LAVATRIANGLE_HEADERS )
  set( LAVATRIANGLE_SOURCES Triangle.cpp )
  set( LAVATRIANGLE_LINK_LIBRARIES
    lava
  )
  common_application( lavaTriangle )

  set( LAVATRIANGLEINDEXED_HEADERS )
  set( LAVATRIANGLEINDEXED_SOURCES TriangleIndexed.cpp )
  set( LAVATRIANGLEINDEXED_LINK_LIBRARIES
    lava
  )
  common_application( lavaTriangleIndexed )

  set( LAVACUBEUV_HEADERS )
  set( LAVACUBEUV_SOURCES CubeUV.cpp )
  set( LAVACUBEUV_LINK_LIBRARIES
    lava
  )
  common_application( lavaCubeUV )

  set( LAVACUBEINDEXED_HEADERS )
  set( LAVACUBEINDEXED_SOURCES CubeIndexed.cpp )
  set( LAVACUBEINDEXED_LINK_LIBRARIES
    lava
  )
  common_application( lavaCubeIndexed )

  set( LAVAFULLQUADNOTEXTURE_HEADERS )
  set( LAVAFULLQUADNOTEXTURE_SOURCES FullQuadNoTexture.cpp )
  set( LAVAFULLQUADNOTEXTURE_LINK_LIBRARIES
    lava
  )
  common_application( lavaFullQuadNoTexture )

  set( LAVACOMPUTEEXAMPLE_HEADERS
    lodepng.h
  )
  set( LAVACOMPUTEEXAMPLE_SOURCES
    lodepng.cpp
    ComputeExample.cpp
  )
  set( LAVACOMPUTEEXAMPLE_LINK_LIBRARIES
    lava
  )
  common_application( lavaComputeExample )

  set( LAVAFULLQUADTEXTURE_HEADERS )
  set( LAVAFULLQUADTEXTURE_SOURCES FullQuadTexture.cpp )
  set( LAVAFULLQUADTEXTURE_LINK_LIBRARIES
    lava
  )
  common_application( lavaFullQuadTexture )

  set( LAVAMESH_HEADERS )
  set( LAVAMESH_SOURCES Mesh.cpp )
  set( LAVAMESH_LINK_LIBRARIES
    lava
  )
  common_application( lavaMesh )

  set( LAVANORMALTEXTURE_HEADERS )
  set( LAVANORMALTEXTURE_SOURCES NormalTexture.cpp )
  set( LAVANORMALTEXTURE_LINK_LIBRARIES
    lava
  )
  common_application( lavaNormalTexture )

  set( LAVAINSTANCING_HEADERS )
  set( LAVAINSTANCING_SOURCES Instancing.cpp )
  set( LAVAINSTANCING_LINK_LIBRARIES
    lava
  )
  common_application( lavaInstancing )

  set( LAVABLUR_HEADERS )
  set( LAVABLUR_SOURCES Blur.cpp )
  set( LAVABLUR_LINK_LIBRARIES
    lava
  )
  common_application( lavaBlur )

endif( )