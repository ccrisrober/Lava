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

  set( LAVADEMO_HEADERS )
  set( LAVADEMO_SOURCES Demo.cpp )
  set( LAVADEMO_LINK_LIBRARIES lava )
  common_application( lavaDemo )

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

  set( LAVACOMPUTESUM_HEADERS )
  set( LAVACOMPUTESUM_SOURCES ComputeSum.cpp )
  set( LAVACOMPUTESUM_LINK_LIBRARIES lava )
  common_application( lavaComputeSum )

  set( LAVACOMPUTEFLOCK_HEADERS )
  set( LAVACOMPUTEFLOCK_SOURCES ComputeFlock.cpp )
  set( LAVACOMPUTEFLOCK_LINK_LIBRARIES lava )
  common_application( lavaComputeFlock )

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

  set( LAVACOMPUTERAYTRACING_HEADERS )
  set( LAVACOMPUTERAYTRACING_SOURCES ComputeRayTracing.cpp )
  set( LAVACOMPUTERAYTRACING_LINK_LIBRARIES lava )
  common_application( lavaComputeRayTracing )

  set( LAVAINPUTCORRECTION_HEADERS )
  set( LAVAINPUTCORRECTION_SOURCES InputCorrection.cpp )
  set( LAVAINPUTCORRECTION_LINK_LIBRARIES lava )
  common_application( lavaInputCorrection )

  set( LAVAINSTANCING_HEADERS )
  set( LAVAINSTANCING_SOURCES Instancing.cpp )
  set( LAVAINSTANCING_LINK_LIBRARIES lava )
  common_application( lavaInstancing )

  set( LAVASKYBOX_HEADERS utils/Camera.h )
  set( LAVASKYBOX_SOURCES Skybox.cpp )
  set( LAVASKYBOX_LINK_LIBRARIES lava )
  common_application( lavaSkybox )

endif( )