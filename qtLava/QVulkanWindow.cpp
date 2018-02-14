#include "QVulkanWindow.h"
#include <QPlatformSurfaceEvent>
#include <qapplication.h>

namespace lava
{
  QVulkanWindowRenderer::~QVulkanWindowRenderer( void )
  {
  }
  void QVulkanWindowRenderer::initResources( void )
  {
  }
  void QVulkanWindowRenderer::initSwapChainResources( void )
  {
  }
  void QVulkanWindowRenderer::releaseSwapChainResources( void )
  {
  }
  void QVulkanWindowRenderer::releaseResources( void )
  {
  }
  QVulkanWindow::QVulkanWindow( QWindow * parent )
    : QWindow( parent )
  {
    setSurfaceType( QSurface::SurfaceType::VulkanSurface );
  }
  QVulkanWindow::~QVulkanWindow( void )
  {
    _device->waitIdle( );
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
  void QVulkanWindow::beginFrame( void )
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

      endFrame( );
    }
  }
  void QVulkanWindow::endFrame( void )
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
    currrentCmd->end( );
    /*vk::Result err =*/ _gfxQueue->submit( SubmitInfo {
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      currrentCmd,
      _renderComplete
    } );

    /*if ( err != vk::Result::eSuccess )
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
    }*/

    _defaultFramebuffer->present( _gfxQueue, _renderComplete );
  }

  void QVulkanWindow::frameReady( void )
  {
    if ( QThread::currentThread( ) != QApplication::instance( )->thread( ) )
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

    endFrame( );
  }
  
  std::shared_ptr<CommandBuffer> QVulkanWindow::currentCommandBuffer( void ) const
  {
    return imageRes[ _defaultFramebuffer->index( ) ].commandBuffer;
  }

  bool QVulkanWindow::setupRenderPass( void )
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

  bool QVulkanWindow::setupFramebuffer( void )
  {
    _defaultFramebuffer.reset( );    // need to be reset, before creating a new one!!
    _defaultFramebuffer.reset( new qt::DefaultFramebuffer( _device, _surface,
      _colorFormat, _colorSpace, _dsFormat, _renderPass ) );
    return true;
  }

  bool QVulkanWindow::setupPipelineCache( void )
  {
    return true;
  }
  std::shared_ptr<PhysicalDevice> QVulkanWindow::physicalDevice( void ) const
  {
    return _physicalDevice;
  }
  const vk::PhysicalDeviceProperties QVulkanWindow::physicalDeviceProperties( void ) const
  {
    return physicalDevice( )->getDeviceProperties( );
  }
  std::shared_ptr<Device> QVulkanWindow::device( void ) const
  {
    return _device;
  }
  std::shared_ptr<Queue> QVulkanWindow::gfxQueue( void ) const
  {
    return _gfxQueue;
  }
  std::shared_ptr<CommandPool> QVulkanWindow::gfxCommandPool( void ) const
  {
    return _cmdPool;
  }
  std::shared_ptr<RenderPass> QVulkanWindow::defaultRenderPass( void ) const
  {
    return _renderPass;
  }
  vk::Format QVulkanWindow::colorFormat( void ) const
  {
    return _colorFormat;
  }
  vk::Format QVulkanWindow::depthStencilFormat( void ) const
  {
    return _dsFormat;
  }
  QVulkanWindowRenderer * QVulkanWindow::createRenderer( void )
  {
    return nullptr;
  }

  vk::SampleCountFlagBits QVulkanWindow::sampleCountFlagBits( void ) const
  {
    return sampleCount;
  }
  
  static struct {
    vk::SampleCountFlagBits mask;
    int count;
  } sampleCounts[] = {
    // keep this sorted by 'count'
    // keep this sorted by 'count'
    { vk::SampleCountFlagBits::e1, 1 },
    { vk::SampleCountFlagBits::e2, 2 },
    { vk::SampleCountFlagBits::e4, 4 },
    { vk::SampleCountFlagBits::e8, 8 },
    { vk::SampleCountFlagBits::e16, 16 },
    { vk::SampleCountFlagBits::e32, 32 },
    { vk::SampleCountFlagBits::e64, 64 }
  };

  void QVulkanWindow::setSampleCountFlagBits( int sampleCount_ )
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

  std::vector<int> QVulkanWindow::supportedSampleCounts( void )
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
  
  /*void QVulkanWindow::setVulkanInstance( 
    const std::shared_ptr< Instance > instance )
  {
    _instance = instance;
  }*/

  void QVulkanWindow::setQVulkanInstance( const std::shared_ptr< Instance > instance )
  {
    _instance = instance;

    VkInstance vki = static_cast< VkInstance >( static_cast< vk::Instance >( *instance ) );
    _qInstance = new QVulkanInstance( );
    _qInstance->setVkInstance( vki );
    if ( _qInstance->create( ) )
    {
      setVulkanInstance( _qInstance );
    }
    else
    {
      std::cerr << "Error: Can't create QVulkanInstance!" << std::endl;
      exit( -1 );
    }
  }

  void QVulkanWindow::init( void )
  {
    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    if ( !renderer )
    {
      renderer = createRenderer( );
    }

    // Find a physical device with presentation support
    assert( _instance->getPhysicalDeviceCount( ) != 0 );
    _physicalDevice = _instance->getPhysicalDevice( 0 );

    if ( !_physicalDevice )
    {
      LAVA_RUNTIME_ERROR( "Failed to find a device with presentation support" );
    }


    _surface = createSurfaceKHR( );
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

    /* TODO _window->_callbackResize = [&]( int w, int h )
    {
      device( )->waitIdle( );
      resize( w, h );
    };*/

    if ( renderer )
    {
      renderer->initResources( );
    }

    initialized = true;
  }

  void QVulkanWindow::reset( void )
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

  void QVulkanWindow::exposeEvent( QExposeEvent* )
  {
    if( isExposed( ) )
    {
      if ( !initialized )
      {
        init( );
      }
      requestUpdate( );
    }
  }

  void QVulkanWindow::resizeEvent( QResizeEvent* )
  {
    std::cout << "Resize" << std::endl;
    if ( _defaultFramebuffer )
    {
      _defaultFramebuffer.reset( );    // need to be reset, before creating a new one!!
      _defaultFramebuffer.reset( new qt::DefaultFramebuffer( _device, _surface,
        _colorFormat, _colorSpace, _dsFormat, _renderPass ) );
    }
  }

  bool QVulkanWindow::event( QEvent* ev )
  {
    QEvent::Type type = ev->type( );
    switch( type )
    {
      case QEvent::UpdateRequest:
        beginFrame( );
        break;
      case QEvent::PlatformSurface:
        /*if (static_cast<QPlatformSurfaceEvent *>(ev)->surfaceEventType() ==
            QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
          //releaseSwapChain();
          reset( );
        }*/
        //init( );
        break;
      default:
        break;
    }
    return QWindow::event( ev );
  }

  vk::Offset2D QVulkanWindow::swapChainImageSize( void ) const
  {
    QSize _size = size( );
    return vk::Offset2D{
      _size.width( ),
      _size.height( ),
    };
  }

}
