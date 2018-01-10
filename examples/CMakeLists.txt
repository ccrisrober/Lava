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

  set( LAVATRIANGLEINDEXED_HEADERS )
  set( LAVATRIANGLEINDEXED_SOURCES TriangleIndexed.cpp )
  set( LAVATRIANGLEINDEXED_LINK_LIBRARIES lava )
  common_application( lavaTriangleIndexed )

  set( LAVAEARTH_HEADERS )
  set( LAVAEARTH_SOURCES Earth.cpp )
  set( LAVAEARTH_LINK_LIBRARIES lava )
  common_application( lavaEarth )

  set( LAVAMATCAP2DARRAY_HEADERS )
  set( LAVAMATCAP2DARRAY_SOURCES Matcap2DArray.cpp )
  set( LAVAMATCAP2DARRAY_LINK_LIBRARIES lava )
  common_application( lavaMatcap2DArray )

  set( LAVASKYBOX_HEADERS utils/Camera.h )
  set( LAVASKYBOX_SOURCES Skybox.cpp )
  set( LAVASKYBOX_LINK_LIBRARIES lava )
  common_application( lavaSkybox )

  set( LAVAMULTISETDESCRIPTOR_HEADERS )
  set( LAVAMULTISETDESCRIPTOR_SOURCES MultiSetDescriptor.cpp )
  set( LAVAMULTISETDESCRIPTOR_LINK_LIBRARIES lava )
  common_application( lavaMultiSetDescriptor )

  set( LAVAPLANARREFLECTION_HEADERS )
  set( LAVAPLANARREFLECTION_SOURCES PlanarReflection.cpp )
  set( LAVAPLANARREFLECTION_LINK_LIBRARIES lava )
  common_application( lavaPlanarReflection )

  set( LAVACLIPPLANE_HEADERS )
  set( LAVACLIPPLANE_SOURCES ClipPlane.cpp )
  set( LAVACLIPPLANE_LINK_LIBRARIES lava )
  common_application( lavaClipPlane )

  set( LAVASTENCILOUTLINE_HEADERS )
  set( LAVASTENCILOUTLINE_SOURCES StencilOutline.cpp )
  set( LAVASTENCILOUTLINE_LINK_LIBRARIES lava )
  common_application( lavaStencilOutline )

  set( LAVADYNAMICUNIFORMBUFFER_HEADERS )
  set( LAVADYNAMICUNIFORMBUFFER_SOURCES DynamicUniformBuffer.cpp )
  set( LAVADYNAMICUNIFORMBUFFER_LINK_LIBRARIES lava )
  common_application( lavaDynamicUniformBuffer )

endif( )