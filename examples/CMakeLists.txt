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

  set( LAVAFOO_HEADERS )
  set( LAVAFOO_SOURCES Foo.cpp )
  set( LAVAFOO_LINK_LIBRARIES lava lavaRenderer lavaUtils )
  common_application( lavaFoo )

## Basic Examples
  set( LAVAVKINFO_HEADERS )
  set( LAVAVKINFO_SOURCES VkInfo.cpp )
  set( LAVAVKINFO_LINK_LIBRARIES lava )
  common_application( lavaVkInfo )

if( GLFW3_FOUND )
  set( LAVACLEARSCREEN_HEADERS )
  set( LAVACLEARSCREEN_SOURCES ClearScreen.cpp )
  set( LAVACLEARSCREEN_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaClearScreen )
endif( )

if(FALSE) # fake a block comment
  set( LAVATRIANGLENOBUFFER_HEADERS )
  set( LAVATRIANGLENOBUFFER_SOURCES TriangleNoBuffer.cpp )
  set( LAVATRIANGLENOBUFFER_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaTriangleNoBuffer )

  set( LAVATRIANGLEINDEXED_HEADERS )
  set( LAVATRIANGLEINDEXED_SOURCES TriangleIndexed.cpp )
  set( LAVATRIANGLEINDEXED_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaTriangleIndexed )
endif( )

if( GLFW3_FOUND )
  set( LAVACUBETEXTURED_HEADERS )
  set( LAVACUBETEXTURED_SOURCES CubeTextured.cpp )
  set( LAVACUBETEXTURED_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaCubeTextured )
endif( )

if(FALSE) # fake a block comment
## Advanced Examples
  set( LAVASAMPLERUNIFORMQUAD_HEADERS )
  set( LAVASAMPLERUNIFORMQUAD_SOURCES SamplerUniformQuad.cpp )
  set( LAVASAMPLERUNIFORMQUAD_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaSamplerUniformQuad )
endif( )
  
  set( LAVASECONDARYBUFFER_HEADERS )
  set( LAVASECONDARYBUFFER_SOURCES SecondaryBuffer.cpp )
  set( LAVASECONDARYBUFFER_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaSecondaryBuffer )

if(FALSE) # fake a block comment
  set( LAVASKYBOX_HEADERS utils/Camera.h )
  set( LAVASKYBOX_SOURCES Skybox.cpp )
  set( LAVASKYBOX_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaSkybox )

  set( LAVAPLANARREFLECTION_HEADERS )
  set( LAVAPLANARREFLECTION_SOURCES PlanarReflection.cpp )
  set( LAVAPLANARREFLECTION_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaPlanarReflection )

  set( LAVAINSTANCING_HEADERS )
  set( LAVAINSTANCING_SOURCES Instancing.cpp )
  set( LAVAINSTANCING_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaInstancing )

###  set( LAVAMATCAP2DARRAY_HEADERS )
###  set( LAVAMATCAP2DARRAY_SOURCES Matcap2DArray.cpp )
###  set( LAVAMATCAP2DARRAY_LINK_LIBRARIES lava lavaRenderer )
###  common_application( lavaMatcap2DArray )

  set( LAVAMESHDERIVATIONSPECIALIZATION_HEADERS )
  set( LAVAMESHDERIVATIONSPECIALIZATION_SOURCES 
    MeshDerivationSpecialization.cpp )
  set( LAVAMESHDERIVATIONSPECIALIZATION_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaMeshDerivationSpecialization )

  set( LAVAEARTH_HEADERS )
  set( LAVAEARTH_SOURCES Earth.cpp )
  set( LAVAEARTH_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaEarth )

  set( LAVAMULTISETDESCRIPTOR_HEADERS )
  set( LAVAMULTISETDESCRIPTOR_SOURCES MultiSetDescriptor.cpp )
  set( LAVAMULTISETDESCRIPTOR_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaMultiSetDescriptor )

  set( LAVASTENCILTOONOUTLINE_HEADERS )
  set( LAVASTENCILTOONOUTLINE_SOURCES StencilToonOutline.cpp )
  set( LAVASTENCILTOONOUTLINE_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaStencilToonOutline )

  set( LAVASWAPQUADTEXTURE_HEADERS )
  set( LAVASWAPQUADTEXTURE_SOURCES SwapQuadTexture.cpp )
  set( LAVASWAPQUADTEXTURE_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaSwapQuadTexture )

## Geometry Examples
  set( LAVAGEOMETRYFIGURES_HEADERS )
  set( LAVAGEOMETRYFIGURES_SOURCES GeometryFigures.cpp )
  set( LAVAGEOMETRYFIGURES_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaGeometryFigures )

  set( LAVAMESHEXPLOSION_HEADERS )
  set( LAVAMESHEXPLOSION_SOURCES MeshExplosion.cpp )
  set( LAVAMESHEXPLOSION_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaMeshExplosion )

  set( LAVAMESHNORMALS_HEADERS )
  set( LAVAMESHNORMALS_SOURCES MeshNormals.cpp )
  set( LAVAMESHNORMALS_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaMeshNormals )

###  set( LAVACLIPPLANE_HEADERS )
###  set( LAVACLIPPLANE_SOURCES ClipPlane.cpp )
###  set( LAVACLIPPLANE_LINK_LIBRARIES lava lavaRenderer )
###  common_application( lavaClipPlane )

###  set( LAVABILLBOARDGEOMETRY_HEADERS utils/Camera.h )
###  set( LAVABILLBOARDGEOMETRY_SOURCES BillboardGeometry.cpp )
###  set( LAVABILLBOARDGEOMETRY_LINK_LIBRARIES lava lavaRenderer )
###  common_application( lavaBillboardGeometry )

###  set( LAVABUTTERFLIESGRASSLAND_HEADERS utils/Camera.h )
###  set( LAVABUTTERFLIESGRASSLAND_SOURCES ButterfliesGrassland.cpp )
###  set( LAVABUTTERFLIESGRASSLAND_LINK_LIBRARIES lava lavaRenderer )
###  common_application( lavaButterfliesGrassland )

## Tesselation examples 
  set( LAVAHEIGHTMAPTESSELATION_HEADERS utils/Camera.h )
  set( LAVAHEIGHTMAPTESSELATION_SOURCES HeightmapTesselation.cpp )
  set( LAVAHEIGHTMAPTESSELATION_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaHeightmapTesselation )

## Compute Examples
  set( LAVACOMPUTESUM_HEADERS )
  set( LAVACOMPUTESUM_SOURCES ComputeSum.cpp )
  set( LAVACOMPUTESUM_LINK_LIBRARIES lava )
  common_application( lavaComputeSum )

  set( LAVACOMPUTECOPY_HEADERS )
  set( LAVACOMPUTECOPY_SOURCES ComputeCopy.cpp )
  set( LAVACOMPUTECOPY_LINK_LIBRARIES lava )
  common_application( lavaComputeCopy )

  set( LAVAFRACTALCOMPUTE_HEADERS )
  set( LAVAFRACTALCOMPUTE_SOURCES FractalCompute.cpp )
  set( LAVAFRACTALCOMPUTE_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaFractalCompute )







  set( LAVASPARKLE_HEADERS )
  set( LAVASPARKLE_SOURCES Sparkle.cpp )
  set( LAVASPARKLE_LINK_LIBRARIES lava lavaRenderer )
  common_application( lavaSparkle )
endif( )

  set( LAVATHREADPOOL_HEADERS )
  set( LAVATHREADPOOL_SOURCES ThreadPool.cpp )
  set( LAVATHREADPOOL_LINK_LIBRARIES lava lavaRenderer lavaUtils )
  common_application( lavaThreadPool )


if( QT5CORE_FOUND )
  set( LAVAQTRENDER_HEADERS )
  set( LAVAQTRENDER_SOURCES QtRender.cpp )
  set( LAVAQTRENDER_LINK_LIBRARIES lava qtLava )
  common_application( lavaQtRender )
endif( )

set( LAVADEFERREDSHADING_HEADERS )
set( LAVADEFERREDSHADING_SOURCES DeferredShading.cpp )
set( LAVADEFERREDSHADING_LINK_LIBRARIES lava lavaRenderer )
common_application( lavaDeferredShading )

set( LAVATEXELBUFFER_HEADERS )
set( LAVATEXELBUFFER_SOURCES TexelBuffer.cpp )
set( LAVATEXELBUFFER_LINK_LIBRARIES lava lavaRenderer )
common_application( lavaTexelBuffer )
endif( )