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

##  set( LAVAVKINFO_HEADERS )
##  set( LAVAVKINFO_SOURCES VkInfo.cpp )
##  set( LAVAVKINFO_LINK_LIBRARIES lava )
##  common_application( lavaVkInfo )
##
##  set( LAVACLEARSCREEN_HEADERS )
##  set( LAVACLEARSCREEN_SOURCES ClearScreen.cpp )
##  set( LAVACLEARSCREEN_LINK_LIBRARIES lava )
##  common_application( lavaClearScreen )
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
##  set( LAVATRIANGLETESSELATION_HEADERS )
##  set( LAVATRIANGLETESSELATION_SOURCES TriangleTesselation.cpp )
##  set( LAVATRIANGLETESSELATION_LINK_LIBRARIES lava )
##  common_application( lavaTriangleTesselation )
##
##  set( LAVACUBEINDEXED_HEADERS )
##  set( LAVACUBEINDEXED_SOURCES CubeIndexed.cpp )
##  set( LAVACUBEINDEXED_LINK_LIBRARIES lava )
##  common_application( lavaCubeIndexed )
##
##  set( LAVACUBEUV_HEADERS )
##  set( LAVACUBEUV_SOURCES CubeUV.cpp )
##  set( LAVACUBEUV_LINK_LIBRARIES lava )
##  common_application( lavaCubeUV )
##
##  set( LAVAINSTANCING_HEADERS )
##  set( LAVAINSTANCING_SOURCES Instancing.cpp )
##  set( LAVAINSTANCING_LINK_LIBRARIES lava )
##  common_application( lavaInstancing )
##
  set( LAVAFRESNEL_HEADERS )
  set( LAVAFRESNEL_SOURCES Fresnel.cpp )
  set( LAVAFRESNEL_LINK_LIBRARIES lava )
  common_application( lavaFresnel )
##
  set( LAVASUBSURFACESCATTERING_HEADERS )
  set( LAVASUBSURFACESCATTERING_SOURCES SubsurfaceScattering.cpp )
  set( LAVASUBSURFACESCATTERING_LINK_LIBRARIES lava )
  common_application( lavaSubsurfaceScattering )
##
##  set( LAVAHEIGHTMAPTESS_HEADERS )
##  set( LAVAHEIGHTMAPTESS_SOURCES HeightmapTess.cpp )
##  set( LAVAHEIGHTMAPTESS_LINK_LIBRARIES lava )
##  common_application( lavaHeightmapTess )
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
##  set( LAVASPECULARLIGHTS_HEADERS )
##  set( LAVASPECULARLIGHTS_SOURCES SpecularLights.cpp )
##  set( LAVASPECULARLIGHTS_LINK_LIBRARIES lava )
##  common_application( lavaSpecularLights )
##
##  set( LAVASTENCILOUTLINE_HEADERS )
##  set( LAVASTENCILOUTLINE_SOURCES StencilOutline.cpp )
##  set( LAVASTENCILOUTLINE_LINK_LIBRARIES lava )
##  common_application( lavaStencilOutline )
##
##  set( LAVAPOSTPROCESS_HEADERS )
##  set( LAVAPOSTPROCESS_SOURCES PostProcess.cpp )
##  set( LAVAPOSTPROCESS_LINK_LIBRARIES lava )
##  common_application( lavaPostProcess )
##
##  set( LAVAMESH_HEADERS )
##  set( LAVAMESH_SOURCES Mesh.cpp )
##  set( LAVAMESH_LINK_LIBRARIES lava )
##  common_application( lavaMesh )
##
##  set( LAVAMESHUV_HEADERS )
##  set( LAVAMESHUV_SOURCES MeshUV.cpp )
##  set( LAVAMESHUV_LINK_LIBRARIES lava )
##  common_application( lavaMeshUV )
##
##  set( LAVAPLANARREFLECTION_HEADERS )
##  set( LAVAPLANARREFLECTION_SOURCES PlanarReflection.cpp )
##  set( LAVAPLANARREFLECTION_LINK_LIBRARIES lava )
##  common_application( lavaPlanarReflection )
##
##  set( LAVAMATCAP_HEADERS )
##  set( LAVAMATCAP_SOURCES Matcap.cpp )
##  set( LAVAMATCAP_LINK_LIBRARIES lava )
##  common_application( lavaMatcap )
##
##  set( LAVAMULTIVIEWPORT_HEADERS )
##  set( LAVAMULTIVIEWPORT_SOURCES MultiViewport.cpp )
##  set( LAVAMULTIVIEWPORT_LINK_LIBRARIES lava )
##  common_application( lavaMultiViewport )
##
##  set( LAVAPUSHCONSTANTS_HEADERS )
##  set( LAVAPUSHCONSTANTS_SOURCES PushConstants.cpp )
##  set( LAVAPUSHCONSTANTS_LINK_LIBRARIES lava )
##  common_application( lavaPushConstants )
##
##  set( LAVASKYBOX_HEADERS utils/Camera.h )
##  set( LAVASKYBOX_SOURCES Skybox.cpp )
##  set( LAVASKYBOX_LINK_LIBRARIES lava )
##  common_application( lavaSkybox )
##
##  set( LAVABLENDINGCUBE_HEADERS )
##  set( LAVABLENDINGCUBE_SOURCES BlendingCube.cpp )
##  set( LAVABLENDINGCUBE_LINK_LIBRARIES lava )
##  common_application( lavaBlendingCube )
##
##  set( LAVASHADOWMAPPING_HEADERS )
##  set( LAVASHADOWMAPPING_SOURCES ShadowMapping.cpp )
##  set( LAVASHADOWMAPPING_LINK_LIBRARIES lava )
##  common_application( lavaShadowMapping )
##
##  set( LAVAGEOMETRY2DFIGURES_HEADERS )
##  set( LAVAGEOMETRY2DFIGURES_SOURCES Geometry2DFigures.cpp )
##  set( LAVAGEOMETRY2DFIGURES_LINK_LIBRARIES lava )
##  common_application( lavaGeometry2DFigures )
##
##  set( LAVACOMPUTERAYTRACING_HEADERS )
##  set( LAVACOMPUTERAYTRACING_SOURCES ComputeRayTracing.cpp )
##  set( LAVACOMPUTERAYTRACING_LINK_LIBRARIES lava )
##  common_application( lavaComputeRayTracing )
##
##  set( LAVACOMPUTEEFFECT_HEADERS )
##  set( LAVACOMPUTEEFFECT_SOURCES ComputeEffect.cpp )
##  set( LAVACOMPUTEEFFECT_LINK_LIBRARIES lava )
##  common_application( lavaComputeEffect )
##
##  set( LAVACOMPUTEUV_HEADERS )
##  set( LAVACOMPUTEUV_SOURCES ComputeUV.cpp )
##  set( LAVACOMPUTEUV_LINK_LIBRARIES lava )
##  common_application( lavaComputeUV )
##
##  set( LAVAGRAVEYARDSCENE_HEADERS )
##  set( LAVAGRAVEYARDSCENE_SOURCES GraveyardScene.cpp )
##  set( LAVAGRAVEYARDSCENE_LINK_LIBRARIES lava )
##  common_application( lavaGraveyardScene )
##
##  set( LAVAFRACTALCOMPUTETEXTURE_HEADERS )
##  set( LAVAFRACTALCOMPUTETEXTURE_SOURCES FractalComputeTexture.cpp )
##  set( LAVAFRACTALCOMPUTETEXTURE_LINK_LIBRARIES lava )
##  common_application( lavaFractalComputeTexture )

  set( LAVAFRACTALCOMPUTETEXTUREFPSLIMIT_HEADERS )
  set( LAVAFRACTALCOMPUTETEXTUREFPSLIMIT_SOURCES FractalComputeTextureFPSLimit.cpp )
  set( LAVAFRACTALCOMPUTETEXTUREFPSLIMIT_LINK_LIBRARIES lava )
  common_application( lavaFractalComputeTextureFPSLimit )

  set( LAVATEXTURE2DARRAY_HEADERS )
  set( LAVATEXTURE2DARRAY_SOURCES Texture2DArray.cpp )
  set( LAVATEXTURE2DARRAY_LINK_LIBRARIES lava )
  common_application( lavaTexture2DArray )

  set( LAVAARRAYSUM_HEADERS )
  set( LAVAARRAYSUM_SOURCES ArraySum.cpp )
  set( LAVAARRAYSUM_LINK_LIBRARIES lava )
  common_application( lavaArraySum )

  set( LAVAHEIGHTMAPTESS2_HEADERS )
  set( LAVAHEIGHTMAPTESS2_SOURCES HeightmapTess2.cpp )
  set( LAVAHEIGHTMAPTESS2_LINK_LIBRARIES lava )
  common_application( lavaHeightmapTess2 )

  set( LAVACLIPPLANE_HEADERS )
  set( LAVACLIPPLANE_SOURCES ClipPlane.cpp )
  set( LAVACLIPPLANE_LINK_LIBRARIES lava )
  common_application( lavaClipPlane )

  set( LAVAPERFECTTOON_HEADERS )
  set( LAVAPERFECTTOON_SOURCES PerfectToon.cpp )
  set( LAVAPERFECTTOON_LINK_LIBRARIES lava )
  common_application( lavaPerfectToon )
endif( )