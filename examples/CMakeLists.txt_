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

  set( LAVATRIANGLENOBUFFER_HEADERS )
  set( LAVATRIANGLENOBUFFER_SOURCES TriangleNoBuffer.cpp )
  set( LAVATRIANGLENOBUFFER_LINK_LIBRARIES lava )
  common_application( lavaTriangleNoBuffer )

  set( LAVASECONDARYBUFFER_HEADERS )
  set( LAVASECONDARYBUFFER_SOURCES SecondaryBuffer.cpp )
  set( LAVASECONDARYBUFFER_LINK_LIBRARIES lava )
  common_application( lavaSecondaryBuffer )

  set( LAVACOMPUTEPARTICLES_HEADERS )
  set( LAVACOMPUTEPARTICLES_SOURCES ComputeParticles.cpp )
  set( LAVACOMPUTEPARTICLES_LINK_LIBRARIES lava )
  common_application( lavaComputeParticles )

  set( LAVAFRACTALCOMPUTE_HEADERS )
  set( LAVAFRACTALCOMPUTE_SOURCES FractalCompute.cpp )
  set( LAVAFRACTALCOMPUTE_LINK_LIBRARIES lava )
  common_application( lavaFractalCompute )

  set( LAVACOMPUTECOPY_HEADERS )
  set( LAVACOMPUTECOPY_SOURCES ComputeCopy.cpp )
  set( LAVACOMPUTECOPY_LINK_LIBRARIES lava )
  common_application( lavaComputeCopy )

  set( LAVAEARTH_HEADERS )
  set( LAVAEARTH_SOURCES Earth.cpp )
  set( LAVAEARTH_LINK_LIBRARIES lava )
  common_application( lavaEarth )

  set( LAVACOMPUTESUM_HEADERS )
  set( LAVACOMPUTESUM_SOURCES ComputeSum.cpp )
  set( LAVACOMPUTESUM_LINK_LIBRARIES lava )
  common_application( lavaComputeSum )

  ##set( LAVAPOSTPROCESS_HEADERS )
  ##set( LAVAPOSTPROCESS_SOURCES PostProcess.cpp )
  ##set( LAVAPOSTPROCESS_LINK_LIBRARIES lava )
  ##common_application( lavaPostProcess )

  set( LAVATRIANGLEINDEXED_HEADERS )
  set( LAVATRIANGLEINDEXED_SOURCES TriangleIndexed.cpp )
  set( LAVATRIANGLEINDEXED_LINK_LIBRARIES lava )
  common_application( lavaTriangleIndexed )

  set( LAVACUBETEXTURED_HEADERS )
  set( LAVACUBETEXTURED_SOURCES CubeTextured.cpp )
  set( LAVACUBETEXTURED_LINK_LIBRARIES lava )
  common_application( lavaCubeTextured )

  set( LAVAMESHDERIVATIONSPECIALIZATION_HEADERS )
  set( LAVAMESHDERIVATIONSPECIALIZATION_SOURCES MeshDerivationSpecialization.cpp )
  set( LAVAMESHDERIVATIONSPECIALIZATION_LINK_LIBRARIES lava )
  common_application( lavaMeshDerivationSpecialization )

  set( LAVAPLANARREFLECTION_HEADERS )
  set( LAVAPLANARREFLECTION_SOURCES PlanarReflection.cpp )
  set( LAVAPLANARREFLECTION_LINK_LIBRARIES lava )
  common_application( lavaPlanarReflection )

  set( LAVASAMPLERUNIFORMQUAD_HEADERS )
  set( LAVASAMPLERUNIFORMQUAD_SOURCES SamplerUniformQuad.cpp )
  set( LAVASAMPLERUNIFORMQUAD_LINK_LIBRARIES lava )
  common_application( lavaSamplerUniformQuad )

  set( LAVAMATCAP2DARRAY_HEADERS )
  set( LAVAMATCAP2DARRAY_SOURCES Matcap2DArray.cpp )
  set( LAVAMATCAP2DARRAY_LINK_LIBRARIES lava )
  common_application( lavaMatcap2DArray )

  set( LAVAINSTANCING_HEADERS )
  set( LAVAINSTANCING_SOURCES Instancing.cpp )
  set( LAVAINSTANCING_LINK_LIBRARIES lava )
  common_application( lavaInstancing )

  set( LAVAMULTIVIEWPORTTOON_HEADERS )
  set( LAVAMULTIVIEWPORTTOON_SOURCES MultiViewportToon.cpp )
  set( LAVAMULTIVIEWPORTTOON_LINK_LIBRARIES lava )
  common_application( lavaMultiViewportToon )

  set( LAVASKYBOX_HEADERS utils/Camera.h )
  set( LAVASKYBOX_SOURCES Skybox.cpp )
  set( LAVASKYBOX_LINK_LIBRARIES lava )
  common_application( lavaSkybox )

  ## Geometry examples
  set( LAVAGEOMETRYFIGURES_HEADERS )
  set( LAVAGEOMETRYFIGURES_SOURCES GeometryFigures.cpp )
  set( LAVAGEOMETRYFIGURES_LINK_LIBRARIES lava )
  common_application( lavaGeometryFigures )

  set( LAVAMESHEXPLOSION_HEADERS )
  set( LAVAMESHEXPLOSION_SOURCES MeshExplosion.cpp )
  set( LAVAMESHEXPLOSION_LINK_LIBRARIES lava )
  common_application( lavaMeshExplosion )

  set( LAVABILLBOARDGEOMETRY_HEADERS utils/Camera.h )
  set( LAVABILLBOARDGEOMETRY_SOURCES BillboardGeometry.cpp )
  set( LAVABILLBOARDGEOMETRY_LINK_LIBRARIES lava )
  common_application( lavaBillboardGeometry )

  set( LAVABUTTERFLIESGRASSLAND_HEADERS utils/Camera.h )
  set( LAVABUTTERFLIESGRASSLAND_SOURCES ButterfliesGrassland.cpp )
  set( LAVABUTTERFLIESGRASSLAND_LINK_LIBRARIES lava )
  common_application( lavaButterfliesGrassland )

  ## Tesselation examples
  set( LAVAHEIGHTMAPTESSELATION_HEADERS utils/Camera.h )
  set( LAVAHEIGHTMAPTESSELATION_SOURCES HeightmapTesselation.cpp )
  set( LAVAHEIGHTMAPTESSELATION_LINK_LIBRARIES lava )
  common_application( lavaHeightmapTesselation )

  set( LAVATEAPOTDEPTH_HEADERS 
    utils/Camera.h
    utils/teapotdata.h
    utils/TeapotQuad.h
  )
  set( LAVATEAPOTDEPTH_SOURCES TeapotDepth.cpp )
  set( LAVATEAPOTDEPTH_LINK_LIBRARIES lava )
  common_application( lavaTeapotDepth )


endif( )