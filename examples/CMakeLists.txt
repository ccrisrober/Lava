########## GLSL2SPIRV ##########
include( glsl2spirv )

compileSPV( )

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

##  set( LAVAINFO_HEADERS )
##  set( LAVAINFO_SOURCES info.cpp )
##  set( LAVAINFO_LINK_LIBRARIES lava )
##  common_application( lavaInfo )
##
##  set( LAVACLEARSCREENDEMO_HEADERS )
##  set( LAVACLEARSCREENDEMO_SOURCES ClearScreenDemo.cpp )
##  set( LAVACLEARSCREENDEMO_LINK_LIBRARIES lava )
##  common_application( lavaClearScreenDemo )
##
##  set( LAVATRIANGLE_HEADERS )
##  set( LAVATRIANGLE_SOURCES Triangle.cpp )
##  set( LAVATRIANGLE_LINK_LIBRARIES lava )
##  common_application( lavaTriangle )
##
##  set( LAVATRIANGLEINDEXED_HEADERS )
##  set( LAVATRIANGLEINDEXED_SOURCES TriangleIndexed.cpp )
##  set( LAVATRIANGLEINDEXED_LINK_LIBRARIES lava )
##  common_application( lavaTriangleIndexed )
##
##  set( LAVACUBEINDEXED_HEADERS materials/RenderNormalMaterial.h )
##  set( LAVACUBEINDEXED_SOURCES CubeIndexed.cpp )
##  set( LAVACUBEINDEXED_LINK_LIBRARIES lava )
##  common_application( lavaCubeIndexed )
##
  set( LAVACUBEUV_HEADERS )
  set( LAVACUBEUV_SOURCES CubeUV.cpp )
  set( LAVACUBEUV_LINK_LIBRARIES lava )
  common_application( lavaCubeUV )
##
##  set( LAVAFULLQUADNOTEXTURE_HEADERS )
##  set( LAVAFULLQUADNOTEXTURE_SOURCES FullQuadNoTexture.cpp )
##  set( LAVAFULLQUADNOTEXTURE_LINK_LIBRARIES lava )
##  common_application( lavaFullQuadNoTexture )
##
##  set( LAVAFULLQUADTEXTURE_HEADERS )
##  set( LAVAFULLQUADTEXTURE_SOURCES FullQuadTexture.cpp )
##  set( LAVAFULLQUADTEXTURE_LINK_LIBRARIES lava )
##  common_application( lavaFullQuadTexture )
##
##  set( LAVAMESH_HEADERS )
##  set( LAVAMESH_SOURCES Mesh.cpp )
##  set( LAVAMESH_LINK_LIBRARIES lava )
##  common_application( lavaMesh )
##
##  set( LAVANORMALTEXTURE_HEADERS )
##  set( LAVANORMALTEXTURE_SOURCES NormalTexture.cpp )
##  set( LAVANORMALTEXTURE_LINK_LIBRARIES lava )
##  common_application( lavaNormalTexture )
##
##  set( LAVAINSTANCING_HEADERS )
##  set( LAVAINSTANCING_SOURCES Instancing.cpp )
##  set( LAVAINSTANCING_LINK_LIBRARIES lava )
##  common_application( lavaInstancing )
##
##  set( LAVAGRASS_HEADERS )
##  set( LAVAGRASS_SOURCES Grass.cpp )
##  set( LAVAGRASS_LINK_LIBRARIES lava )
##  common_application( lavaGrass )
##
##  set( LAVABUTTERFLIES_HEADERS )
##  set( LAVABUTTERFLIES_SOURCES Butterflies.cpp )
##  set( LAVABUTTERFLIES_LINK_LIBRARIES lava )
##  common_application( lavaButterflies )
##
  set( LAVASPHERICALHARMONICS_HEADERS materials/SHMaterial.h )
  set( LAVASPHERICALHARMONICS_SOURCES SphericalHarmonics.cpp )
  set( LAVASPHERICALHARMONICS_LINK_LIBRARIES lava )
  common_application( lavaSphericalHarmonics )
##
##  set( LAVASHOWNORMALS_HEADERS )
##  set( LAVASHOWNORMALS_SOURCES ShowNormals.cpp )
##  set( LAVASHOWNORMALS_LINK_LIBRARIES lava )
##  common_application( lavaShowNormals )
##
##  set( LAVAHEIGHTMAP_HEADERS )
##  set( LAVAHEIGHTMAP_SOURCES Heightmap.cpp )
##  set( LAVAHEIGHTMAP_LINK_LIBRARIES lava )
##  common_application( lavaHeightmap )
##  
##  set( LAVAHEIGHTMAPTESS_HEADERS )
##  set( LAVAHEIGHTMAPTESS_SOURCES HeightmapTess.cpp )
##  set( LAVAHEIGHTMAPTESS_LINK_LIBRARIES lava )
##  common_application( lavaHeightmapTess )
##
##  set( LAVARAYTRACINGFRAG_HEADERS )
##  set( LAVARAYTRACINGFRAG_SOURCES RayTracingFrag.cpp )
##  set( LAVARAYTRACINGFRAG_LINK_LIBRARIES lava )
##  common_application( lavaRayTracingFrag )
##
##  set( LAVAMATCAP_HEADERS )
##  set( LAVAMATCAP_SOURCES Matcap.cpp )
##  set( LAVAMATCAP_LINK_LIBRARIES lava )
##  common_application( lavaMatcap )
##  
##  set( LAVASPECIALIZATIONCONSTANT_HEADERS )
##  set( LAVASPECIALIZATIONCONSTANT_SOURCES SpecializationConstant.cpp )
##  set( LAVASPECIALIZATIONCONSTANT_LINK_LIBRARIES lava )
##  common_application( lavaSpecializationConstant )
##
##  set( LAVAPUSHCONSTANT_HEADERS )
##  set( LAVAPUSHCONSTANT_SOURCES PushConstant.cpp )
##  set( LAVAPUSHCONSTANT_LINK_LIBRARIES lava )
##  common_application( lavaPushConstant )
##
##  set( LAVAGEOMETRYEXPLOSION_HEADERS )
##  set( LAVAGEOMETRYEXPLOSION_SOURCES GeometryExplosion.cpp )
##  set( LAVAGEOMETRYEXPLOSION_LINK_LIBRARIES lava )
##  common_application( lavaGeometryExplosion )
##
##  set( LAVASWAPQUADTEXTURE_HEADERS )
##  set( LAVASWAPQUADTEXTURE_SOURCES SwapQuadTexture.cpp )
##  set( LAVASWAPQUADTEXTURE_LINK_LIBRARIES lava )
##  common_application( lavaSwapQuadTexture )

  set( LAVAVERTEXDISPLACEMENT_HEADERS )
  set( LAVAVERTEXDISPLACEMENT_SOURCES VertexDisplacement.cpp )
  set( LAVAVERTEXDISPLACEMENT_LINK_LIBRARIES lava )
  common_application( lavaVertexDisplacement )

  set( LAVAFOO_HEADERS )
  set( LAVAFOO_SOURCES Foo.cpp )
  set( LAVAFOO_LINK_LIBRARIES lava )
  common_application( lavaFoo )
  

##
##
##
  ##set( LAVACOMPUTEEXAMPLE_HEADERS
  ##  lodepng.h
  ##)
  ##set( LAVACOMPUTEEXAMPLE_SOURCES
  ##  lodepng.cpp
  ##  ComputeExample.cpp
  ##)
  ##set( LAVACOMPUTEEXAMPLE_LINK_LIBRARIES
  ##  lava
  ##)
  ##common_application( lavaComputeExample )
##
##
##
##
  ####set( LAVABLUR_HEADERS )
  ####set( LAVABLUR_SOURCES Blur.cpp )
  ####set( LAVABLUR_LINK_LIBRARIES
  ####  lava
  ####)
  ####common_application( lavaBlur )
##
##
  ##set( LAVASKYBOX_HEADERS )
  ##set( LAVASKYBOX_SOURCES Skybox.cpp )
  ##set( LAVASKYBOX_LINK_LIBRARIES
  ##  lava
  ##)
  ##common_application( lavaSkybox )
##
##
##
##
##
##
##
  ####set( LAVAPIPELINES_HEADERS )
  ####set( LAVAPIPELINES_SOURCES Pipelines.cpp )
  ####set( LAVAPIPELINES_LINK_LIBRARIES
  ####  lava
  ####)
  ####common_application( lavaPipelines )
##
##
  ####set( LAVARAYTRACING_HEADERS )
  ####set( LAVARAYTRACING_SOURCES RayTracing.cpp )
  ####set( LAVARAYTRACING_LINK_LIBRARIES
  ####  lava
  ####)
  ####common_application( lavaRayTracing )
##
  ##
##
##
##
##
  ##
##
##
##
##
##
  ####set( LAVAFIREPARTICLES_HEADERS )
  ####set( LAVAFIREPARTICLES_SOURCES FireParticles.cpp )
  ####set( LAVAFIREPARTICLES_LINK_LIBRARIES
  ####  lava
  ####)
  ####common_application( lavaFireParticles )
  ##
##
##
  ####set( LAVACOMPUTECONVOLUTION_HEADERS )
  ####set( LAVACOMPUTECONVOLUTION_SOURCES ComputeConvolution.cpp )
  ####set( LAVACOMPUTECONVOLUTION_LINK_LIBRARIES
  ####  lava
  ####)
  ####common_application( lavaComputeConvolution )
  ##
##
##
  ####set( LAVARAYTRACERCOMPUTER_HEADERS )
  ####set( LAVARAYTRACERCOMPUTER_SOURCES RayTracerComputer.cpp )
  ####set( LAVARAYTRACERCOMPUTER_LINK_LIBRARIES
  ####  lava
  ####)
  ####common_application( lavaRayTracerComputer )
  
endif( )