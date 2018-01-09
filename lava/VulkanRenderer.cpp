#include "VulkanRenderer.h"
#include "engine/Clock.h"
#include "Log.h"

#include <thread>

namespace lava
{
  RenderAPICapabilities VulkanWindow::caps( void ) const
	{
		return _caps;
	}

  void VulkanWindow::resizeEvent( uint32_t w, uint32_t h )
  {
    if ( _defaultFramebuffer )
    {
      _defaultFramebuffer.reset( );    // need to be reset, before creating a new one!!
      _defaultFramebuffer.reset( new DefaultFramebuffer( _device, _surface,
        _colorFormat, _colorSpace, _dsFormat, _renderPass ) );
    }
  }

  void VulkanWindow::show( void )
	{
		if( !_initialized )
		{
			init( );
			_initialized = true;

      // TODO: savePipelineCache( );
		}
    //float elapsed = 0.0f;
    //uint32_t frames = 0;
    //engine::Clock clock;
		// TODO: SHOW WINDOW
    while ( _window->isRunning( ) )
    {
      //const auto elapsedTime = clock.reset( );
      _window->pollEvents( );
      beginFrame( );
      //endFrame( );

      //elapsed += elapsedTime;
      //++frames;

      //if ( elapsed >= 1.0f )
      //{
      //  Log::info( "FPS: ", std::to_string( ( frames / elapsed ) * 0.1f ) );
      //  frames = 0;
      //  elapsed = 0.0f;
      //}

      // Very crude method to prevent your GPU from overheating.
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
	}

	void VulkanWindow::setVulkanInstance( const std::shared_ptr< Instance > instance )
	{
    _instance = instance;
	}

  VulkanWindowRenderer* VulkanWindow::createRenderer( void )
  {
    return nullptr;
  }

  void VulkanWindow::frameReady( void )
  {
    endFrame( );
  }

  vk::SampleCountFlagBits VulkanWindow::sampleCountFlagBits( void ) const
  {
    return sampleCount;
  }

  static struct
  {
    vk::SampleCountFlagBits mask;
    int count;
  } sampleCounts[ ] = {
    // keep this sorted by 'count'
    { vk::SampleCountFlagBits::e1, 1 },
    { vk::SampleCountFlagBits::e2, 2 },
    { vk::SampleCountFlagBits::e4, 4 },
    { vk::SampleCountFlagBits::e8, 8 },
    { vk::SampleCountFlagBits::e16, 16 },
    { vk::SampleCountFlagBits::e32, 32 },
    { vk::SampleCountFlagBits::e64, 64 }
  };

  void VulkanWindow::setSampleCountFlagBits( int sampleCount_ )
  {
    for ( uint32_t i = 0, l = sizeof( sampleCounts ) / sizeof( sampleCounts[ 0 ] ); i < l; ++i )
    {
      if ( sampleCounts[ i ].count == sampleCount_ )
      {
        sampleCount = sampleCounts[ i ].mask;
        return;
      }
    }
  }

  std::vector<int> VulkanWindow::supportedSampleCounts( void )
  {
    auto limits = physicalDevice( )->getDeviceProperties( ).limits;
    vk::SampleCountFlags color = limits.framebufferColorSampleCounts;
    vk::SampleCountFlags depth = limits.framebufferDepthSampleCounts;
    vk::SampleCountFlags stencil = limits.framebufferStencilSampleCounts;

    std::vector< int > result;

    for ( uint32_t i = 0, l = sizeof( sampleCounts ) / sizeof(sampleCounts[ 0 ] );
      i < l; ++i )
    {
      if ( ( color & sampleCounts[ i ].mask )
        && ( depth & sampleCounts[ i ].mask )
        && ( stencil & sampleCounts[ i ].mask ) )
      {
        result.push_back( sampleCounts[ i ].count );
      }
    }
    return result;
  }

  int VulkanWindow::imagesCount( void ) const
  {
    return _defaultFramebuffer->imagesCount( );
  }

  void VulkanWindow::setDeviceExtensions( 
    const std::vector< std::string >& exts )
  {
    _requestedDeviceExts = exts;
  }

  void VulkanWindow::initCapabilites( void )
	{
    const auto& deviceProps = _physicalDevice->getDeviceProperties( );
    const auto& deviceFeatures = _physicalDevice->getDeviceFeatures( );
    const vk::PhysicalDeviceLimits& deviceLimits = deviceProps.limits;

    DriverVersion driverVersion;
    driverVersion.major = ( ( uint32_t ) ( deviceProps.apiVersion ) >> 22 );
    driverVersion.minor = ( ( uint32_t ) ( deviceProps.apiVersion ) >> 12 ) & 0x3ff;
    driverVersion.release = ( uint32_t ) ( deviceProps.apiVersion ) & 0xfff;
    driverVersion.build = 0;

    _caps.setDriverVersion( driverVersion );
    _caps.setDeviceName( deviceProps.deviceName );

    // Determine vendor
    switch ( deviceProps.vendorID )
    {
      case 0x10DE:
        std::cout << "GPU NVIDIA" << std::endl;
        _caps.setVendor( GPU_NVIDIA );
        break;
      case 0x1002:
        std::cout << "GPU AMD" << std::endl;
        _caps.setVendor( GPU_AMD );
        break;
      case 0x163C:
      case 0x8086:
        std::cout << "GPU INTEL" << std::endl;
        _caps.setVendor( GPU_INTEL );
        break;
      default:
        std::cout << "GPU UNKNOWN" << std::endl;
        _caps.setVendor( GPU_UNKNOWN );
        break;
    };

    if ( deviceFeatures.textureCompressionBC )
    {
      _caps.setCapability( RSC_TEXTURE_COMPRESSION_BC );
    }

    if ( deviceFeatures.textureCompressionETC2 )
    {
      _caps.setCapability( RSC_TEXTURE_COMPRESSION_ETC2 );
    }

    if ( deviceFeatures.textureCompressionASTC_LDR )
    {
      _caps.setCapability( RSC_TEXTURE_COMPRESSION_ASTC );
    }

    _caps.setMaxBoundVertexBuffers( deviceLimits.maxVertexInputBindings );
    _caps.setNumMultiRenderTargets( deviceLimits.maxColorAttachments );

    _caps.setCapability( RSC_COMPUTE_PROGRAM );

    _caps.setNumTextureUnits( GpuProgramType::FRAGMENT_PROGRAM,
      deviceLimits.maxPerStageDescriptorSampledImages );
    _caps.setNumTextureUnits( GpuProgramType::VERTEX_PROGRAM,
      deviceLimits.maxPerStageDescriptorSampledImages );
    _caps.setNumTextureUnits( GpuProgramType::COMPUTE_PROGRAM,
      deviceLimits.maxPerStageDescriptorSampledImages );

    _caps.setNumGpuParamBlockBuffers( GpuProgramType::FRAGMENT_PROGRAM,
      deviceLimits.maxPerStageDescriptorUniformBuffers );
    _caps.setNumGpuParamBlockBuffers( GpuProgramType::VERTEX_PROGRAM,
      deviceLimits.maxPerStageDescriptorUniformBuffers );
    _caps.setNumGpuParamBlockBuffers( GpuProgramType::COMPUTE_PROGRAM,
      deviceLimits.maxPerStageDescriptorUniformBuffers );

    _caps.setNumLoadStoreTextureUnits( GpuProgramType::FRAGMENT_PROGRAM,
      deviceLimits.maxPerStageDescriptorStorageImages );
    _caps.setNumLoadStoreTextureUnits( GpuProgramType::COMPUTE_PROGRAM,
      deviceLimits.maxPerStageDescriptorStorageImages );

    if ( deviceFeatures.geometryShader )
    {
      _caps.setCapability( RSC_GEOMETRY_PROGRAM );
      _caps.addShaderProfile( "gs_5_0" );
      _caps.setNumTextureUnits( GpuProgramType::GEOMETRY_PROGRAM,
        deviceLimits.maxPerStageDescriptorSampledImages );
      _caps.setNumGpuParamBlockBuffers( GpuProgramType::GEOMETRY_PROGRAM,
        deviceLimits.maxPerStageDescriptorUniformBuffers );
      _caps.setGeometryProgramNumOutputVertices(
        deviceLimits.maxGeometryOutputVertices );
    }

    if ( deviceFeatures.tessellationShader )
    {
      _caps.setCapability( RSC_TESSELLATION_PROGRAM );

      _caps.setNumTextureUnits( GpuProgramType::TESS_EVAL_PROGRAM,
        deviceLimits.maxPerStageDescriptorSampledImages );
      _caps.setNumTextureUnits( GpuProgramType::TESS_CTRL_PROGRAM,
        deviceLimits.maxPerStageDescriptorSampledImages );

      _caps.setNumGpuParamBlockBuffers( GpuProgramType::TESS_EVAL_PROGRAM,
        deviceLimits.maxPerStageDescriptorUniformBuffers );
      _caps.setNumGpuParamBlockBuffers( GpuProgramType::TESS_CTRL_PROGRAM,
        deviceLimits.maxPerStageDescriptorUniformBuffers );
    }

    _caps.setNumCombinedTextureUnits(
      _caps.getNumTextureUnits( GpuProgramType::FRAGMENT_PROGRAM )
      + _caps.getNumTextureUnits( GpuProgramType::VERTEX_PROGRAM ) +
      _caps.getNumTextureUnits( GpuProgramType::GEOMETRY_PROGRAM )
      + _caps.getNumTextureUnits( GpuProgramType::TESS_EVAL_PROGRAM ) +
      _caps.getNumTextureUnits( GpuProgramType::TESS_CTRL_PROGRAM )
      + _caps.getNumTextureUnits( GpuProgramType::COMPUTE_PROGRAM ) );

    _caps.setNumCombinedGpuParamBlockBuffers(
      _caps.getNumGpuParamBlockBuffers( GpuProgramType::FRAGMENT_PROGRAM )
      + _caps.getNumGpuParamBlockBuffers( GpuProgramType::VERTEX_PROGRAM ) +
      _caps.getNumGpuParamBlockBuffers( GpuProgramType::GEOMETRY_PROGRAM )
      + _caps.getNumGpuParamBlockBuffers( GpuProgramType::TESS_EVAL_PROGRAM ) +
      _caps.getNumGpuParamBlockBuffers( GpuProgramType::TESS_CTRL_PROGRAM )
      + _caps.getNumGpuParamBlockBuffers( GpuProgramType::COMPUTE_PROGRAM ) );

    _caps.setNumCombinedLoadStoreTextureUnits(
      _caps.getNumLoadStoreTextureUnits( GpuProgramType::FRAGMENT_PROGRAM )
      + _caps.getNumLoadStoreTextureUnits( GpuProgramType::COMPUTE_PROGRAM ) );

    _caps.addShaderProfile( "glsl" );
	}

  void VulkanWindow::setWindowTitle( const char*& name )
  {
    setWindowTitle( std::string( name ) );
  }

  void VulkanWindow::setWindowTitle( const std::string& name )
  {
    _window->setWindowTitle( name );
  }

	void VulkanWindow::init( void )
	{
    _window = std::make_shared< Window >( "Title", 
      _swapChainImageSize.x, _swapChainImageSize.y );

    // Find a physical device with presentation support
    assert( _instance->getPhysicalDeviceCount( ) != 0 );
    VkInstance inst = VkInstance( vk::Instance( *_instance ) );
    for ( size_t i = 0, s = _instance->getPhysicalDeviceCount( ); i < s; ++i )
    {
      VkPhysicalDevice pd( vk::PhysicalDevice( *_instance->getPhysicalDevice( i ) ) );
      if ( glfwGetPhysicalDevicePresentationSupport( inst, pd, 0 ) )
      {
        _physicalDevice = _instance->getPhysicalDevice( i );
        break;
      }
    }
    if ( !_physicalDevice )
    {
      LAVA_RUNTIME_ERROR( "Failed to find a device with presentation support" );
    }

    if ( !renderer )
    {
      renderer = createRenderer( );
    }

		//initCapabilites( );
    _surface = _instance->createSurfaceKHR( _window->getWindow( ) );
    auto surfaceFormats = _physicalDevice->getSurfaceFormats( _surface );
    assert( !surfaceFormats.empty( ) );
    uint32_t numFormats = surfaceFormats.size( );

    bool gamma = false;
    // If there is no preferred format, use standard RGBA
    if ( ( numFormats == 1 )
      && ( surfaceFormats[ 0 ].format == vk::Format::eUndefined ) )
    {
      if ( gamma )
      {
        _colorFormat = vk::Format::eR8G8B8A8Srgb;
      }
      else
      {
        _colorFormat = vk::Format::eB8G8R8A8Unorm;
      }

      _colorSpace = surfaceFormats[0].colorSpace;
    }
    else
    {
      bool foundFormat = false;

      std::vector<vk::Format> wantedFormatsUNORM =
      {
        vk::Format::eR8G8B8A8Unorm,
        vk::Format::eA8B8G8R8UnormPack32,
        vk::Format::eB8G8R8Unorm
      };

      std::vector<vk::Format> wantedFormatsSRGB =
      {
        vk::Format::eB8G8R8A8Srgb,
        vk::Format::eA8B8G8R8SrgbPack32,
        vk::Format::eB8G8R8Srgb
      };

      std::vector<vk::Format> wantedFormats;
      if ( gamma )
      {
        wantedFormats = wantedFormatsSRGB;
      }
      else
      {
        wantedFormats = wantedFormatsUNORM;
      }

      for ( const auto& wantedFormat : wantedFormats )
      {
        for ( const auto& surfFormat : surfaceFormats )
        {
          if ( surfFormat.format == wantedFormat )
          {
            _colorFormat = surfFormat.format;
            _colorSpace = surfFormat.colorSpace;

            foundFormat = true;
            break;
          }
        }
        if ( foundFormat )
        {
          break;
        }
      }

      wantedFormatsSRGB.clear( );
      wantedFormatsUNORM.clear( );
      wantedFormats.clear( );

      // If we haven't found anything, fall back to first available
      if ( !foundFormat )
      {
        _colorFormat = surfaceFormats[ 0 ].format;
        _colorSpace = surfaceFormats[0].colorSpace;

        if ( gamma )
        {
          throw new std::runtime_error( R"(Cannot find a valid sRGB format for a render window surface, falling back to a default format.)" );
        }
      }
    }
    VkBool32 validDepthFormat = lava::utils::getSupportedDepthFormat( 
      _physicalDevice, _dsFormat );
    assert( validDepthFormat );

    // Search for a graphics queue and a present queue in the array of 
    //    queue families, try to find one that supports both

    std::vector<vk::QueueFamilyProperties> queueFamilyIndices =
      _physicalDevice->getQueueFamilyProperties( );
    assert( !queueFamilyIndices.empty( ) );

    _gfxQueueFamilyIdx = uint32_t( -1 );
    _presQueueFamilyIdx = uint32_t( -1 );

    auto phyDev = static_cast< vk::PhysicalDevice >( *_physicalDevice );
    for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
    {
      VkBool32 presentSupport = phyDev.getSurfaceSupportKHR( i, *_surface );

      /*printf( "queue family %d: flags=0x%x count=%d supportsPresent=%d\n", 
        queueFamilyIndices[ i ].queueFlags, queueFamilyIndices[ i ].queueCount,
        presentSupport == VK_TRUE ? 1 : 0 );*/

      if ( _gfxQueueFamilyIdx == uint32_t( -1 ) && 
        ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics ) 
        && presentSupport )
      {
        _gfxQueueFamilyIdx = i;
      }
    }

    if ( _gfxQueueFamilyIdx != uint32_t(-1))
    {
      _presQueueFamilyIdx = _gfxQueueFamilyIdx;
    }
    else {
      std::cerr << "No queue with graphics+present; trying separate queues" << std::endl;
      for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
      {
        if ( _gfxQueueFamilyIdx == uint32_t( -1 ) &&
          ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics ) )
        {
          _gfxQueueFamilyIdx = i;
        }

        VkBool32 presentSupport = phyDev.getSurfaceSupportKHR( i, *_surface );
        if ( _presQueueFamilyIdx == uint32_t( -1 ) && presentSupport )
        {
          _presQueueFamilyIdx = i;
        }
      }
    }
    if ( _gfxQueueFamilyIdx == uint32_t(-1))
    {
      std::cerr << "ERROR: No graphics queue family found" << std::endl;
      throw;
    }
    if ( _presQueueFamilyIdx == uint32_t(-1))
    {
      std::cerr << "ERROR: No present queue family found" << std::endl;
      throw;
    }

    /*std::vector<uint32_t> queueFamilyIndices =
      _physicalDevice->getGraphicsPresentQueueFamilyIndices( _surface );
    assert( !queueFamilyIndices.empty( ) );
    _gfxQueueFamilyIdx = queueFamilyIndices[ 0 ];*/

    // Create a new device with the VK_KHR_SWAPCHAIN_EXTENSION enabled.
    std::vector<std::string> enabledLayerNames;

    std::vector<std::string> enabledExtensionNames;

    _requestedDeviceExts.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    
    for ( const auto& ext : _requestedDeviceExts )
    {
      if ( _physicalDevice->extensionSupported( ext ) )
      {
        enabledExtensionNames.push_back( ext );
      }
    }

    vk::DeviceQueueCreateInfo dqci;
    dqci.setQueueFamilyIndex( _gfxQueueFamilyIdx );

    const float queuePriority = 0.0f;
    dqci.setQueueCount( 1 );
    dqci.setPQueuePriorities( &queuePriority );
    std::vector< vk::DeviceQueueCreateInfo > queueCreateInfos;
    queueCreateInfos.push_back( dqci );
    
    
    if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx )
    {
      vk::DeviceQueueCreateInfo dqci2;
      dqci2.setQueueFamilyIndex( _presQueueFamilyIdx );
      dqci2.setQueueCount( 1 );
      dqci2.setPQueuePriorities( &queuePriority );

      queueCreateInfos.push_back( dqci2 );
    }

    _device = _physicalDevice->createDevice( 
      queueCreateInfos, 
      enabledLayerNames,
      enabledExtensionNames,
      _physicalDevice->getDeviceFeatures( )
    );

    _gfxQueue = _device->getQueue( _gfxQueueFamilyIdx, 0 );

    if ( _gfxQueueFamilyIdx == _presQueueFamilyIdx )
    {
      _presQueue = _gfxQueue;
    }
    else
    {
      _presQueue = _device->getQueue( _presQueueFamilyIdx, 0 );
    }

    _cmdPool = _device->createCommandPool(
      {}, //vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
      _gfxQueueFamilyIdx );

    if( !createDefaultRenderPass( ) )
    {
      return;
    }

    _renderComplete = _device->createSemaphore( );
    //createPipelineCache( );

    _defaultFramebuffer.reset( );    // need to be reset, before creating a new one!!
    _defaultFramebuffer.reset( new DefaultFramebuffer( _device, _surface,
      _colorFormat, _colorSpace, _dsFormat, _renderPass ) );

    _window->_callbackResize = [&]( int w, int h )
    {
      device( )->waitIdle( );
      resize( w, h );
    };

    if ( renderer )
    {
      renderer->initResources( );
    }
	}

	void VulkanWindow::reset( void )
	{
    if ( !_device ) return;
    _device->waitIdle( );

    if ( renderer )
    {
      renderer->releaseResources( );
      _device->waitIdle( );
    }

    if ( _renderPass )
    {
      _renderPass.reset();
    }

    if ( _cmdPool )
    {
      _cmdPool.reset( );
    }




    if ( _device )
    {
      _device.reset( );
    }
	}

  void VulkanWindow::beginFrame( void )
  {
    _defaultFramebuffer->acquireNextFrame( );

    imageRes[ _defaultFramebuffer->index( ) ].commandBuffer = _cmdPool->allocateCommandBuffer( );

    imageRes[ _defaultFramebuffer->index( ) ].commandBuffer->beginSimple( );

    if ( renderer )
    {
      renderer->nextFrame( );
    }
    else
    {
      std::array<vk::ClearValue, 2 > clearValues;
      std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
      clearValues[ 0 ].color = vk::ClearColorValue( ccv );
      clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

      const glm::ivec2 size = swapChainImageSize( );
      auto cmd = currentCommandBuffer( );
      vk::Rect2D rect;
      rect.extent.width = size.x;
      rect.extent.height = size.y;
      cmd->beginRenderPass(
        defaultRenderPass( ),
        currentFramebuffer( ),
        rect, clearValues, vk::SubpassContents::eInline
      );

      cmd->endRenderPass( );

      endFrame( );
    }
  }

  void VulkanWindow::endFrame( void )
  {
    if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx )
    {
      // Add the swapchain image release to the command buffer that will be submitted to the graphics queue.
      vk::ImageSubresourceRange issr;
      issr.aspectMask = vk::ImageAspectFlagBits::eColor;
      issr.levelCount = issr.layerCount = 1;
      ImageMemoryBarrier barrier( {}, vk::AccessFlagBits::eColorAttachmentWrite, 
        vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::ePresentSrcKHR, 
        _gfxQueueFamilyIdx, _presQueueFamilyIdx, nullptr, issr );
      
      currentCommandBuffer( )->pipelineBarrier( 
        vk::PipelineStageFlagBits::eColorAttachmentOutput, 
        vk::PipelineStageFlagBits::eBottomOfPipe, {}, {}, {}, { barrier } );
    }
    currentCommandBuffer( )->end( );
    vk::Result err = _gfxQueue->submit( SubmitInfo {
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      currentCommandBuffer(),
      _renderComplete
    } );

    if ( err != vk::Result::eSuccess )
    {
      if ( err == vk::Result::eErrorOutOfDateKHR )
      {
        // recreateSwapChain and update
        return;
      }
      else if ( err != vk::Result::eSuboptimalKHR )
      {
        if ( !checkDeviceLost( err ) )
        {
          std::cerr << "Failed to present : " << err << std::endl;
        }
        return;
      }
    }

    _defaultFramebuffer->present( _gfxQueue, _renderComplete );
  }

  bool VulkanWindow::checkDeviceLost( vk::Result err )
  {
    if ( err == vk::Result::eErrorDeviceLost )
    {
      if ( renderer )
      {
        renderer->logicalDeviceLost( );
      }

      //releaseSwapChain( );
      //reset( );
      return true;
    }
    return false;
  }


  /*void VulkanWindow::createPipelineCache( void )
  {
    pipelineCache = _device->createPipelineCache( 0, nullptr );
  }
  void VulkanWindow::savePipelineCache( void )
  {
  }*/

  CommandBufferPtr VulkanWindow::currentCommandBuffer( void ) const
  {
    return imageRes[ _defaultFramebuffer->index( ) ].commandBuffer;
  }

  bool VulkanWindow::createDefaultRenderPass( void )
  {
    const bool msaa = sampleCount > vk::SampleCountFlagBits::e1;

    // TODO: Add MSAA
    std::vector< vk::AttachmentDescription > attDesc =
    {
      vk::AttachmentDescription( // attachment 0 (color render target)
        { }, _colorFormat, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, // color
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, // stencil
        vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
      ),
      vk::AttachmentDescription( // attachment 1 (depth render target)
        { }, _dsFormat, sampleCount,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, // depth
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, // stencil
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
      )
    };

    if ( msaa )
    {
      attDesc.push_back( vk::AttachmentDescription( // attachment 2 (msaa render target)
        { }, _colorFormat, sampleCount,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal
      ) );
    }

    vk::AttachmentReference colorRef( 0, 
      vk::ImageLayout::eColorAttachmentOptimal );
    vk::AttachmentReference depthRef( 1, 
      vk::ImageLayout::eDepthStencilAttachmentOptimal );

    vk::AttachmentReference resolveRef( 0, 
      vk::ImageLayout::eColorAttachmentOptimal );


    vk::SubpassDescription subPassDesc(
      vk::SubpassDescriptionFlags( ),
      vk::PipelineBindPoint::eGraphics,
      0, nullptr,           // input attachments ( count, data )
      1, &colorRef,         // color attachments ( count, data )
      nullptr,              // resolve attachments ( data )
      &depthRef,            // depth attachment ( data )
      0, nullptr            // preserve attachments ( count, data )
    );

    if ( msaa )
    {
      colorRef.attachment = 2;
      subPassDesc.pResolveAttachments = &resolveRef;
    }

    _renderPass = _device->createRenderPass( attDesc, subPassDesc, { } );

    return true;
  }

  void VulkanWindow::recreateSwapChain( void )
  {
  }

	std::shared_ptr< PhysicalDevice > VulkanWindow::physicalDevice( void ) const
	{
		return _physicalDevice;
	}

  const vk::PhysicalDeviceProperties VulkanWindow::physicalDeviceProperties( void ) const
  {
    return physicalDevice( )->getDeviceProperties( );
  }

	std::shared_ptr< Device > VulkanWindow::device( void ) const
	{
		return _device;
	}

	std::shared_ptr< Queue > VulkanWindow::graphicQueue( void ) const
	{
		return _gfxQueue;
	}

	std::shared_ptr< CommandPool > VulkanWindow::graphicsCommandPool( void ) const
	{
		return _cmdPool;
	}

  std::shared_ptr<RenderPass> VulkanWindow::defaultRenderPass( void ) const
  {
    return _renderPass;
  }

	vk::Format VulkanWindow::colorFormat( void ) const
	{
		return _colorFormat;
	}
	
	vk::Format VulkanWindow::depthStencilFormat( void ) const
	{
		return _dsFormat;
	}
	
	glm::ivec2 VulkanWindow::swapChainImageSize( void ) const
	{
		return _swapChainImageSize;
	}

  int VulkanWindow::currentFrame( void ) const
  {
    return _currentFrame;
  }

  int VulkanWindow::swapChainImageCount( void ) const
  {
    return 0;
  }

  int VulkanWindow::currentSwapChainImageIndex( void ) const
  {
    return 0;
  }

  std::shared_ptr<Image> VulkanWindow::swapChainImage( int idx ) const
  {
    return std::shared_ptr<Image>( );
  }

  std::shared_ptr<ImageView> VulkanWindow::swapChainImageView( int idx ) const
  {
    return std::shared_ptr<ImageView>( );
  }

  std::shared_ptr<Image> VulkanWindow::depthStencilImage( void ) const
  {
    return std::shared_ptr<Image>( );
  }

  std::shared_ptr<ImageView> VulkanWindow::depthStencilImageView( void ) const
  {
    return std::shared_ptr<ImageView>( );
  }

  glm::mat4 VulkanWindow::clipCorrectionMatrix( void )
	{
		if ( _clipCorrect == glm::mat4( 1.0f ) )
		{
			_clipCorrect = glm::mat4( 
				1.0f,  0.0f,  0.0f,  0.0f,
				0.0f, -1.0f,  0.0f,  0.0f,
				0.0f,  0.0f,  0.5f,  0.5f,
				0.0f,  0.0f,  0.0f,  1.0f
			);
		}
		return _clipCorrect;
	}
  VulkanWindow::~VulkanWindow( void )
  {
    _device->waitIdle( );
    if ( _window )
    {
      _window.reset( );
    }
    if ( _device )
    {
      if ( imageRes[ 0 ].commandBuffer )
      {
        imageRes[ 0 ].commandBuffer.reset( );
      }
      if ( imageRes[ 1 ].commandBuffer )
      {
        imageRes[ 1 ].commandBuffer.reset( );
      }
      if ( _cmdPool )
      {
        _cmdPool.reset( );
      }
      if ( _renderComplete )
      {
        _renderComplete.reset( );
      }
      if ( _presQueue )
      {
        _presQueue.reset( );
      }
      if ( _gfxQueue )
      {
        _gfxQueue.reset( );
      }
      if ( _defaultFramebuffer )
      {
        _defaultFramebuffer.reset( );
      }
      if ( _renderPass )
      {
        _renderPass.reset( );
      }
      if ( _surface )
      {
        _surface.reset( );
      }
      if ( _device )
      {
        _device.reset( );
      }
    }

    if ( _physicalDevice )
    {
      _physicalDevice.reset( );
    }
    /*if ( _instance )
    {
      _instance.reset( );
    }*/
  }
}