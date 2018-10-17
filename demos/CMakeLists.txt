if( POMPEII_WITH_EXAMPLES )
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


  set( POMPEIIVKINFO_HEADERS )
  set( POMPEIIVKINFO_SOURCES VkInfo.cpp )
  set( POMPEIIVKINFO_LINK_LIBRARIES pompeii )
  common_application( pompeiiVkInfo )


  add_subdirectory( compute )
  add_subdirectory( render )
  
  if( QT5CORE_FOUND_ )
    #add_subdirectory( qtRender )
    ##add_subdirectory( qtStencilOutline )
    #add_subdirectory( qtSkybox )
    #add_subdirectory( qtMaskTransition )
    ##add_subdirectory( qtSampleUniform )
    ##add_subdirectory( qtInstancing )
    ##add_subdirectory( qtHeightmapTesselation )
  endif( )

  if( GLFW3_FOUND_ )
    add_subdirectory( glfwMeshInterpolator )
    #$#$#add_subdirectory( glfwParticles )
    
    add_subdirectory( glfwSilhouette )
    
    add_subdirectory( glfwComputeCullLOD )
    
    add_subdirectory( glfwShaderToy )
    
    add_subdirectory( glfwShadowMapping )
    
    add_subdirectory( glfwPostProcessing )
    
    add_subdirectory( glfwSwapTexture )
    
    add_subdirectory( glfwTriangle )
    
    #add_subdirectory( glfwMipmapping )
    
    add_subdirectory( glfwMultisample )
    
    add_subdirectory( glfwRenderToTexture )
    add_subdirectory( glfw3DTexture )
    
    add_subdirectory( glfwMultiViewport )
    
    #$#$#add_subdirectory( glfwImgui )
    
    #add_subdirectory( glfwCubes )

    #add_subdirectory( glfwPoints )

    ######add_subdirectory( glfwPassthrough )

    #$#$#add_subdirectory( glfwWater )
    
    #add_subdirectory( glfwGrassland )
  
    add_subdirectory( glfwMatcap2DArray )
    
    add_subdirectory( glfwInputPostprocess )

    add_subdirectory( glfwGears )

    #add_subdirectory( glfwClipPlane )
    #add_subdirectory( glfwEarth )
    
    #add_subdirectory( glfwBillboards )
    
    #add_subdirectory( glfwTexelBuffer )
    
    ##add_subdirectory( glfwFractalCompute )

    #add_subdirectory( glfwGeometryFigures )
    #add_subdirectory( glfwMeshExplosion )
    #add_subdirectory( glfwMeshNormals )
    #add_subdirectory( glfwMultiSetDescriptor )
    #add_subdirectory( glfwPlanarReflection )
    add_subdirectory( glfwRender )
    #add_subdirectory( glfwSparkle )
    #add_subdirectory( glfwSpecializationConstants )
    ##add_subdirectory( glfwThreadPool )
    #add_subdirectory( glfwTriangleNoBuffer )
    
    ####add_subdirectory( glfwDeferredSubpass )
  endif( )

endif( )