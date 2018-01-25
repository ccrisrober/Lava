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

  set( LAVACLEARSCREEN_HEADERS )
  set( LAVACLEARSCREEN_SOURCES ClearScreen.cpp )
  set( LAVACLEARSCREEN_LINK_LIBRARIES lava )
  common_application( lavaClearScreen )

  set( LAVATRIANGLE_HEADERS )
  set( LAVATRIANGLE_SOURCES Triangle.cpp )
  set( LAVATRIANGLE_LINK_LIBRARIES lava )
  common_application( lavaTriangle )
  
  set( LAVATRIANGLEBYJGM_HEADERS )
  set( LAVATRIANGLEBYJGM_SOURCES TriangleByJGM.cpp )
  set( LAVATRIANGLEBYJGM_LINK_LIBRARIES lava )
  common_application( lavaTriangleByJGM )

  set( LAVATRIANGLEINDEXED_HEADERS )
  set( LAVATRIANGLEINDEXED_SOURCES TriangleIndexed.cpp )
  set( LAVATRIANGLEINDEXED_LINK_LIBRARIES lava )
  common_application( lavaTriangleIndexed )

  set( LAVATRIANGLETESSELATION_HEADERS )
  set( LAVATRIANGLETESSELATION_SOURCES TriangleTesselation.cpp )
  set( LAVATRIANGLETESSELATION_LINK_LIBRARIES lava )
  common_application( lavaTriangleTesselation )
  
  set( LAVAVERTEXBUFFERSBYJGM_HEADERS )
  set( LAVAVERTEXBUFFERSBYJGM_SOURCES VertexBuffersByJGM.cpp )
  set( LAVAVERTEXBUFFERSBYJGM_LINK_LIBRARIES lava )
  common_application( lavaVertexBuffersByJGM )

  set( LAVACUBEINDEXED_HEADERS )
  set( LAVACUBEINDEXED_SOURCES CubeIndexed.cpp )
  set( LAVACUBEINDEXED_LINK_LIBRARIES lava )
  common_application( lavaCubeIndexed )

  set( LAVACUBEUV_HEADERS )
  set( LAVACUBEUV_SOURCES CubeUV.cpp )
  set( LAVACUBEUV_LINK_LIBRARIES lava )
  common_application( lavaCubeUV )

  set( LAVAINSTANCING_HEADERS )
  set( LAVAINSTANCING_SOURCES Instancing.cpp )
  set( LAVAINSTANCING_LINK_LIBRARIES lava )
  common_application( lavaInstancing )

  set( LAVAFULLQUADNOTEXTURE_HEADERS )
  set( LAVAFULLQUADNOTEXTURE_SOURCES FullQuadNoTexture.cpp )
  set( LAVAFULLQUADNOTEXTURE_LINK_LIBRARIES lava )
  common_application( lavaFullQuadNoTexture )

  set( LAVAFULLQUADTEXTURE_HEADERS )
  set( LAVAFULLQUADTEXTURE_SOURCES FullQuadTexture.cpp )
  set( LAVAFULLQUADTEXTURE_LINK_LIBRARIES lava )
  common_application( lavaFullQuadTexture )

  set( LAVASPECULARLIGHTS_HEADERS )
  set( LAVASPECULARLIGHTS_SOURCES SpecularLights.cpp )
  set( LAVASPECULARLIGHTS_LINK_LIBRARIES lava )
  common_application( lavaSpecularLights )

  set( LAVAMESH_HEADERS )
  set( LAVAMESH_SOURCES Mesh.cpp )
  set( LAVAMESH_LINK_LIBRARIES lava )
  common_application( lavaMesh )

  set( LAVAMATCAP_HEADERS )
  set( LAVAMATCAP_SOURCES Matcap.cpp )
  set( LAVAMATCAP_LINK_LIBRARIES lava )
  common_application( lavaMatcap )

  set( LAVASKYBOX_HEADERS utils/Camera.h )
  set( LAVASKYBOX_SOURCES Skybox.cpp )
  set( LAVASKYBOX_LINK_LIBRARIES lava )
  common_application( lavaSkybox )

  set( LAVAGEOMETRY2DFIGURES_HEADERS )
  set( LAVAGEOMETRY2DFIGURES_SOURCES Geometry2DFigures.cpp )
  set( LAVAGEOMETRY2DFIGURES_LINK_LIBRARIES lava )
  common_application( lavaGeometry2DFigures )

  set( LAVACOMPUTERAYTRACING_HEADERS )
  set( LAVACOMPUTERAYTRACING_SOURCES ComputeRayTracing.cpp )
  set( LAVACOMPUTERAYTRACING_LINK_LIBRARIES lava )
  common_application( lavaComputeRayTracing )

  set( LAVACOMPUTEUV_HEADERS )
  set( LAVACOMPUTEUV_SOURCES ComputeUV.cpp )
  set( LAVACOMPUTEUV_LINK_LIBRARIES lava )
  common_application( lavaComputeUV )

  set( LAVAARRAYSUM_HEADERS )
  set( LAVAARRAYSUM_SOURCES ArraySum.cpp )
  set( LAVAARRAYSUM_LINK_LIBRARIES lava )
  common_application( lavaArraySum )
endif( )