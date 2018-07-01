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


  add_subdirectory( compute )
  
  if( QT5CORE_FOUND )
    add_subdirectory( qtRender )
    add_subdirectory( qtStencilOutline )
    add_subdirectory( qtSkybox )
    add_subdirectory( qtSampleUniform )
    add_subdirectory( qtInstancing )
    add_subdirectory( qtHeightmapTesselation )
  endif( )

  if( GLFW3_FOUND )
    add_subdirectory( glfwRender )
    add_subdirectory( glfwEarth )
    add_subdirectory( glfwPlanarReflection )
    add_subdirectory( glfwFractalCompute )
    add_subdirectory( glfwGeometryFigures )
    add_subdirectory( glfwMeshExplosion )
    add_subdirectory( glfwMeshNormals )
    add_subdirectory( glfwSparkle )
  endif( )

endif( )