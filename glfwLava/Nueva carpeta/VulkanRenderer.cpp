/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "VulkanRenderer.h"
#include <thread>

namespace lava
{
  std::thread::id main_thread_id = std::this_thread::get_id( );
  VulkanWindowRenderer::~VulkanWindowRenderer( void )
  {
  }

  void VulkanWindowRenderer::initResources( void )
  {
  }

  void VulkanWindowRenderer::initSwapChainResources( void )
  {
  }

  void VulkanWindowRenderer::releaseSwapChainResources( void )
  {
  }

  void VulkanWindowRenderer::releaseResources( void )
  {
  }

  RenderAPICapabilities VulkanWindow::caps( void ) const
	{
		return _caps;
	}

  void VulkanWindow::resizeEvent( uint32_t /*w*/, uint32_t /*h*/ )
  {
    if ( _defaultFramebuffer )
    {
      if ( renderer )
      {
        renderer->releaseSwapChainResources( );
      }
      _defaultFramebuffer.reset( );    // need to be reset, before creating a new one!!
      _defaultFramebuffer.reset( new glfw::DefaultFramebuffer( _device, _surface,
        _colorFormat, _colorSpace, _dsFormat, _renderPass ) );
      if ( renderer )
      {
        renderer->initSwapChainResources( );
      }
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
    while ( _window->isRunning( ) )
    {
      _window->pollEvents( );
      beginFrame( );

      // Very crude method to prevent your GPU from overheating.
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
	}

  VulkanWindowRenderer* VulkanWindow::createRenderer( void )
  {
    return nullptr;
  }

	void VulkanWindow::setVulkanInstance( 
    const std::shared_ptr< Instance > instance )
	{
    _instance = instance;
	}

  bool VulkanWindow::supportsGrab( void ) const
  {
    return _defaultFramebuffer->supportsGrab( );
  }
  // TODO: frameReady
  void VulkanWindow::requestUpdate( std::shared_ptr<Semaphore> sem )
  {
    if ( main_thread_id != std::this_thread::get_id( ) )
    {
      std::cerr << "You only can called this in main thread" << std::endl;
      return;
    }
    // TODO: Check only called by main thread std::this_thread::
    if ( !_framePending )
    {
      throw "framePending() called without calling nextFrame( )";
    }
    _framePending = false;

    endFrame( sem != nullptr ? 
      sem : defaultFramebuffer( )->getPresentSemaphore( ) );
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
    for ( uint32_t i = 0, l = sizeof( sampleCounts ) / 
      sizeof( sampleCounts[ 0 ] ); i < l; ++i )
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
    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    _window = std::make_shared< Window >( "Title",
      _swapChainImageSize.x, _swapChainImageSize.y );

    if ( !renderer )
    {
      renderer = createRenderer( );
    }

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

		//initCapabilites( );
    _surface = createSurfaceKHR( _window->getWindow( ) );
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
    std::cout << "Finding queue with graphic supporting ... " << std::endl;
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
        std::cout << "\tQueue graphic bit supported." << std::endl;
        if ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eCompute )
        {
          std::cout << "\tQueue compute bit supported." << std::endl;
        }
        if ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eTransfer )
        {
          std::cout << "\tQueue transfer bit supported." << std::endl;
        }
        if ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eSparseBinding )
        {
          std::cout << "\tQueue sparse binding bit supported." << std::endl;
        }
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

    // TODO: PushDescriptorKHR
    // TODO: enabledExtensionNames.push_back( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME );
    // TODO: PushDescriptorKHR

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

    auto enabledFeatures = _physicalDevice->getDeviceFeatures( );
    getEnabledFeatures( enabledFeatures );

    //VkPhysicalDeviceFeatures enabledFeature
    //VkPhysicalDeviceFeatures deviceFeatures;

    _device = _physicalDevice->createDevice( 
      queueCreateInfos, 
      enabledLayerNames,
      enabledExtensionNames,
      enabledFeatures
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
      { }, //vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
      _gfxQueueFamilyIdx );

    if( !setupRenderPass( ) )
    {
      return;
    }

    _renderComplete = _device->createSemaphore( );
    //createPipelineCache( );

    setupFramebuffer( );

    setupPipelineCache( );

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
    if ( !_defaultFramebuffer->_swapchain || _framePending ) return;

    _defaultFramebuffer->acquireNextFrame( );

    size_t idx = _defaultFramebuffer->index( );

    if ( imageRes[ idx ].commandBuffer )
    {
      imageRes[ idx ].commandBuffer.reset( ); // Reset command buffer
      imageRes[ idx ].commandBuffer = nullptr;
    }

    imageRes[ idx ].commandBuffer = _cmdPool->allocateCommandBuffer( );
    imageRes[ idx ].commandBuffer->begin( );

    if ( _framePending )
    {
      _frameGrabTargetImage = _device->createImage( { },
        vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm,
        vk::Extent3D( _window->getWidth( ), _window->getHeight( ), 1 ), 
        1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear, 
        vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, { }, 
        vk::ImageLayout::eUndefined, vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent
      );
    }

    if ( renderer )
    {
      _framePending = true;
      renderer->nextFrame( );
    }
    else
    {
      std::array<vk::ClearValue, 2 > clearValues;
      std::array<float, 4> ccv = { 1.0f, 0.0f, 0.0f, 1.0f };
      clearValues[ 0 ].color = vk::ClearColorValue( ccv );
      clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

      const vk::Offset2D size = swapChainImageSize( );
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

      endFrame( _defaultFramebuffer->getPresentSemaphore( ) );
    }
  }

  void VulkanWindow::endFrame( std::shared_ptr<Semaphore> sem )
  {
    auto currrentCmd = imageRes[ _defaultFramebuffer->index( ) ].commandBuffer;
    if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx )
    {
      // Add the swapchain image release to the command buffer that will be submitted to the graphics queue.
      vk::ImageSubresourceRange issr;
      issr.aspectMask = vk::ImageAspectFlagBits::eColor;
      issr.levelCount = issr.layerCount = 1;
      ImageMemoryBarrier presTrans( {}, vk::AccessFlagBits::eColorAttachmentWrite,
        vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::ePresentSrcKHR, 
        _gfxQueueFamilyIdx, _presQueueFamilyIdx, nullptr, issr );
      
      currrentCmd->pipelineBarrier(
        vk::PipelineStageFlagBits::eColorAttachmentOutput, 
        vk::PipelineStageFlagBits::eBottomOfPipe, {}, {}, {}, { presTrans } );
    }
    // When recording a frame, we need to add a readback at the end and 
    //  skip image presentation
    if ( _frameRecord )
    {
      addReadback( );
    }

    currrentCmd->end( );
    vk::Result err = _gfxQueue->submit( SubmitInfo {
      { sem },//{ _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      currrentCmd,
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
        //renderer->logicalDeviceLost( );
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

  std::shared_ptr<CommandBuffer> VulkanWindow::currentCommandBuffer( void ) const
  {
    if ( !_framePending )
    {
      throw "Attemped to call currentFramebuffer( ) without a active frame";
    }
    return imageRes[ _defaultFramebuffer->index( ) ].commandBuffer;
  }

  bool VulkanWindow::setupRenderPass( void )
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

  bool lava::VulkanWindow::setupFramebuffer( void )
  {
    _defaultFramebuffer.reset( );    // need to be reset, before creating a new one!!
    _defaultFramebuffer.reset( new glfw::DefaultFramebuffer( _device, _surface,
      _colorFormat, _colorSpace, _dsFormat, _renderPass ) );
    return true;
  }

  bool VulkanWindow::setupPipelineCache( void )
  {
    return true;
  }

  std::shared_ptr< Image > VulkanWindow::recordImage( void )
  {
    if( !_defaultFramebuffer->_swapchain )
    {
      throw;
    }
    if ( _framePending )
    {
      throw;
    }
    if ( !_defaultFramebuffer->supportsGrab( ) )
    {
      throw;
    }
    _frameRecord = true;
    beginFrame( );

    return _frameGrabTargetImage;
  }

  void VulkanWindow::recreateSwapChain( void )
  {
  }

  void VulkanWindow::addReadback( void )
  {
    auto frameGrabImage = device( )->createImage( { }, vk::ImageType::e2D,
      vk::Format::eR8G8B8A8Unorm, _frameGrabTargetImage->extent( ), 1, 1,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear, 
      vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, { }, 
      vk::ImageLayout::eUndefined, vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent );
    // Do the actual blit from the swapchain image to our host visible 
    //      destination image
    auto copyCmd = gfxCommandPool( )->allocateCommandBuffer( );
    auto image = defaultFramebuffer( )->getLastImage( );
    copyCmd->begin( );
    ImageMemoryBarrier imr( vk::AccessFlagBits::eMemoryRead, 
      vk::AccessFlagBits::eTransferRead, vk::ImageLayout::ePresentSrcKHR,
      vk::ImageLayout::eTransferSrcOptimal, { }, { }, image, 
      vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 ) );
    copyCmd->pipelineBarrier( vk::PipelineStageFlagBits::eTopOfPipe, 
      vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, imr );

    imr.oldLayout = vk::ImageLayout::ePreinitialized;
    imr.newLayout = vk::ImageLayout::eTransferDstOptimal;
    imr.srcAccessMask = { };
    imr.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    imr.image = frameGrabImage;
    copyCmd->pipelineBarrier( vk::PipelineStageFlagBits::eTopOfPipe,
      vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, imr );

    vk::ImageCopy imageCopyRegion{ };
    imageCopyRegion.srcSubresource.aspectMask =
      vk::ImageAspectFlagBits::eColor;
    imageCopyRegion.srcSubresource.layerCount = 1;
    imageCopyRegion.dstSubresource.aspectMask =
      vk::ImageAspectFlagBits::eColor;
    imageCopyRegion.dstSubresource.layerCount = 1;
    imageCopyRegion.extent.width = _window->getWidth( );
    imageCopyRegion.extent.height = _window->getHeight( );
    imageCopyRegion.extent.depth = 1;
    copyCmd->copyImage( 
      image, vk::ImageLayout::eTransferSrcOptimal, 
      frameGrabImage, vk::ImageLayout::eTransferDstOptimal, 
      imageCopyRegion );

    imr.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    imr.newLayout = vk::ImageLayout::eGeneral;
    imr.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    imr.dstAccessMask = vk::AccessFlagBits::eHostRead;
    imr.image = frameGrabImage;

    copyCmd->pipelineBarrier( vk::PipelineStageFlagBits::eTopOfPipe, 
      vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, imr );
  }
  void VulkanWindow::finishBlockingReadback( void )
  {
    auto image = defaultFramebuffer( )->getLastImage( );

    //Fence::waitForFences( );
    //Fence::resetFences( );

    vk::ImageSubresource isr( vk::ImageAspectFlagBits::eColor, 0, 0 );
    vk::SubresourceLayout srl;

    vk::Device dev = static_cast< vk::Device > ( *device( ) );

    dev.getImageSubresourceLayout(
      static_cast< vk::Image >( *frameGrabImage ), &isr, &srl
    );

    // Map image memory so we can start copying from it
    const char* data = ( const char* ) dev.mapMemory( frameGrabImage->imageMemory, 0,
      VK_WHOLE_SIZE, { } );
    data += srl.offset;



  }

  std::shared_ptr< Image > VulkanWindow::grab( void )
  {
    if ( !_defaultFramebuffer )
    {
      throw "Try to call grab() without a swapchain";
    }
    if ( _framePending )
    {
      throw "Try to call grab() while a frame is still pending";
    }
    // TODO !swapChaing->supportReadback( );

    _framePending = true;
    beginFrame( );

    return _frameGrabTargetImage;
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

	std::shared_ptr< Queue > VulkanWindow::gfxQueue( void ) const
	{
		return _gfxQueue;
	}

	std::shared_ptr< CommandPool > VulkanWindow::gfxCommandPool( void ) const
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
	
	vk::Offset2D VulkanWindow::swapChainImageSize( void ) const
	{
		return _swapChainImageSize;
	}

  int VulkanWindow::currentFrame( void ) const
  {
    if ( !_framePending )
    {
      throw "Attemped to call currentFrame( ) without a active frame";
    }
    return _currentFrame;
  }

  int VulkanWindow::swapChainImageCount( void ) const
  {
    return 0; // TODO
  }

  int VulkanWindow::currentSwapChainImageIndex( void ) const
  {
    if ( !_framePending )
    {
      throw "Attemped to call currentSwapChainImageIndex( ) without a active frame";
    }
    return 0; // TODO
  }

  std::shared_ptr<Image> VulkanWindow::swapChainImage( int /*idx*/ ) const
  {
    return std::shared_ptr<Image>( ); // TODO
  }

  std::shared_ptr<ImageView> VulkanWindow::swapChainImageView( int /*idx*/ ) const
  {
    return std::shared_ptr<ImageView>( ); // TODO
  }

  std::shared_ptr<Image> VulkanWindow::depthStencilImage( void ) const
  {
    return std::shared_ptr<Image>( ); // TODO
  }

  std::shared_ptr<ImageView> VulkanWindow::depthStencilImageView( void ) const
  {
    return std::shared_ptr<ImageView>( ); // TODO
  }

  /*glm::mat4 VulkanWindow::clipCorrectionMatrix( void )
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
	}*/
  VulkanWindow::~VulkanWindow( void )
  {
    _device->waitIdle( );
    if ( _window )
    {
      _window.reset( );
    }
    if ( renderer )
    {
      renderer->releaseResources( );
      _device->waitIdle( );
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

    delete renderer;
  }
}