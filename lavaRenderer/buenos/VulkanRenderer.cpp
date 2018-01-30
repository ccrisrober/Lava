#include "VulkanRenderer.h"

#include <lava/Instance.h>
#include <lava/utils.hpp>

#include <thread>

namespace lava
{
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

  void VulkanWindow::beginFrame( void )
  {
    if ( !_swapChain || _framePending ) return;




    RenderPassResources& resources = renderPassResources[ _currentFrame ];
    const uint32_t maxUint = std::numeric_limits<uint32_t>::max( );
    lava::Fence::waitForFences( { resources.cmdFence }, VK_TRUE, maxUint );
    lava::Fence::resetFences( { resources.cmdFence } );

    if ( resources.cmdBuffer )
    {
      resources.cmdBuffer.reset( ); // Reset command buffer
      resources.cmdBuffer = nullptr;
    }

    resources.cmdBuffer = _commandPool->allocateCommandBuffer( );
    resources.cmdBuffer->begin( );

    if ( _renderer )
    {
      _framePending = true;
      _renderer->nextFrame( );
    }
    else
    {
      std::array<vk::ClearValue, 2 > clearValues;
      std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
      clearValues[ 0 ].color = vk::ClearColorValue( ccv );
      clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

      auto cmd = currentCommandBuffer( );

      const glm::ivec2 size = swapChainImageSize( );
      vk::Rect2D rect;
      rect.extent.width = size.x;
      rect.extent.height = size.y;

      cmd->beginRenderPass(
        defaultRenderPass( ),
        currentFramebuffer( ),
        rect, clearValues, vk::SubpassContents::eInline
      );

      endFrame( );
    }
  }

  void VulkanWindow::endFrame( void )
  {


    if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx )
    {
      // Add the swapchain image release to the command buffer that will be 
      //  submitted to the graphics queue.
      vk::ImageSubresourceRange issr;
      issr.aspectMask = vk::ImageAspectFlagBits::eColor;
      issr.levelCount = issr.layerCount = 1;
      ImageMemoryBarrier presTrans( { }, vk::AccessFlagBits::eColorAttachmentWrite,
        vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::ePresentSrcKHR,
        _gfxQueueFamilyIdx, _presQueueFamilyIdx, nullptr, issr );

      renderPassResources[ _currentFrame ].cmdBuffer->pipelineBarrier( 
        vk::PipelineStageFlagBits::eColorAttachmentOutput, 
        vk::PipelineStageFlagBits::eBottomOfPipe, { }, { }, { }, presTrans );
    }
    renderPassResources[ _currentFrame ].cmdBuffer->end( );


    _currentFrame = ( _currentFrame + 1 ) % 2;
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
    _device->waitIdle( );
    auto surfaceCaps = _physicalDevice->getSurfaceCapabilities( _surface );

    vk::Extent2D extent;
    // If width/height is 0xFFFFFFFF, we can manually specify width, height
    if ( surfaceCaps.currentExtent.width != uint32_t( -1 ) )
    {
      extent = surfaceCaps.currentExtent;
    }
    else
    {
      vk::Extent2D actualExtent = { 1, 1 };

      actualExtent.width = std::max( surfaceCaps.minImageExtent.width,
        std::min( surfaceCaps.maxImageExtent.width, actualExtent.width ) );
      actualExtent.height = std::max( surfaceCaps.minImageExtent.height,
        std::min( surfaceCaps.maxImageExtent.height, actualExtent.height ) );

      extent = actualExtent;
    }

    // Find present mode
    auto presentModes = vk::PhysicalDevice(
      *_device->getPhysicalDevice( ) ).getSurfacePresentModesKHR( *_surface );
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

    uint32_t numImages = surfaceCaps.minImageCount;

    vk::SurfaceTransformFlagBitsKHR surfaceTransform =
      ( surfaceCaps.supportedTransforms &
        vk::SurfaceTransformFlagBitsKHR::eIdentity ) ?
      vk::SurfaceTransformFlagBitsKHR::eIdentity :
      surfaceCaps.currentTransform;

    // Find a supported composite alpha format (not all devices support alpha opaque)
    vk::CompositeAlphaFlagBitsKHR compositeAlpha =
      vk::CompositeAlphaFlagBitsKHR::eOpaque;
    // Simply select the first composite alpha format available
    std::vector<vk::CompositeAlphaFlagBitsKHR> compositeAlphaFlags =
    {
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
      vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
      vk::CompositeAlphaFlagBitsKHR::eInherit,
    };
    for ( auto& compositeAlphaFlag : compositeAlphaFlags )
    {
      if ( surfaceCaps.supportedCompositeAlpha & compositeAlphaFlag )
      {
        compositeAlpha = compositeAlphaFlag;
        break;
      };
    }

    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;

    _swapChainSupportsReadBack = !!( surfaceCaps.supportedUsageFlags &
      vk::ImageUsageFlagBits::eTransferSrc );

    if ( _swapChainSupportsReadBack )
    {
      usage |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    vk::SharingMode sharingMode;
    if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx )
    {

    }
    else
    {
      sharingMode = vk::SharingMode::eExclusive;
    }

    auto newSwapChain = _device->createSwapchain(
      _surface, numImages, _colorFormat, _colorSpace, extent, 1, usage,
      sharingMode, { }, surfaceTransform, compositeAlpha,
      presentMode, true, _swapChain
    );

    if ( _swapChain )
    {
      releaseSwapChain( );
    }
    _swapChain = newSwapChain;








    // depth/stencil buffer
    // assert that a depth and/or stencil format is requested
    auto formatProps = _physicalDevice->getFormatProperties( _dsFormat );
    assert( ( formatProps.linearTilingFeatures &
      vk::FormatFeatureFlagBits::eDepthStencilAttachment ) ||
      ( formatProps.optimalTilingFeatures &
        vk::FormatFeatureFlagBits::eDepthStencilAttachment ) );

    vk::ImageTiling tiling = ( formatProps.optimalTilingFeatures
      & vk::FormatFeatureFlagBits::eDepthStencilAttachment )
      ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear;

    _depthImage = _device->createImage(
      { }, vk::ImageType::e2D, _dsFormat,
      vk::Extent3D( extent.width, extent.height, 1 ), 1, 1,
      vk::SampleCountFlagBits::e1, tiling,
      vk::ImageUsageFlagBits::eDepthStencilAttachment | 
        vk::ImageUsageFlagBits::eTransferSrc,
      vk::SharingMode::eExclusive, { },
      vk::ImageLayout::eUndefined, { } );

    // determine ImageAspect based on format
    vk::ImageAspectFlags aspectFlags;
    if ( _dsFormat != vk::Format::eS8Uint )
    {
      aspectFlags |= vk::ImageAspectFlagBits::eDepth;
    }

    // add eStencil if image contains stencil
    static std::vector<vk::Format> const stencilFormats =
    {
      vk::Format::eD16UnormS8Uint, vk::Format::eD24UnormS8Uint,
      vk::Format::eD32SfloatS8Uint, vk::Format::eS8Uint
    };
    if ( std::find( stencilFormats.begin( ), stencilFormats.end( ),
      _dsFormat ) != stencilFormats.end( ) )
    {
      aspectFlags |= vk::ImageAspectFlagBits::eStencil;
    }

    _depthView = _depthImage->createImageView(
      vk::ImageViewType::e2D, _dsFormat,
      vk::ComponentMapping(
        vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA ),
      vk::ImageSubresourceRange( aspectFlags, 0, 1, 0, 1 ) );


    // TODO: Create msaa images
    const bool msaa = sampleCount > vk::SampleCountFlagBits::e1;


    auto colorImages = _swapChain->getImages( );
    size_t ll = colorImages.size( );

    for ( uint32_t i = 0; i < ll; ++i )
    {
      renderPassResources[ i ].image = colorImages[ i ];
      renderPassResources[ i ].imageView = 
        colorImages[ i ]->createImageView(
            vk::ImageViewType::e2D,
            _colorFormat,
            {
              vk::ComponentSwizzle::eR,
              vk::ComponentSwizzle::eG,
              vk::ComponentSwizzle::eB,
              vk::ComponentSwizzle::eA
            },
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
          );


      std::vector<std::shared_ptr<ImageView>> imageViews( msaa ? 3 : 2 );
      imageViews[ 0 ] = renderPassResources[ i ].imageView;
      imageViews[ 1 ] = _depthView;
      /*if ( msaa )
      {
        imageViews[2] = 
      }*/


      renderPassResources[ i ].framebuffer = 
        _device->createFramebuffer( _renderPass, imageViews, extent, 1 );
    }

    std::cout << "Framebuffer Swapchain OK" << std::endl;


    for ( size_t i = 0; i < 2; ++i )
    {
      renderPassResources[ i ].cmdFence = _device->createFence( true );
    }

    if ( _renderer )
    {
      _renderer->initSwapChainResources( );
    }
  }

  void VulkanWindow::releaseSwapChain( void )
  {
    if ( !_swapChain )
    {
      return;
    }

    _device->waitIdle( );
    if ( _renderer )
    {
      _renderer->releaseSwapChainResources( );
      _device->waitIdle( );
    }
  }

  void VulkanWindow::init( void )
  {
    _window = std::make_shared<Window>( "Title", 
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
        //getEnabledFeatures( );  // TODO
        break;
      }
    }
    if ( !_physicalDevice )
    {
      LAVA_RUNTIME_ERROR( "Failed to find a device with presentation support" );
    }




    if ( !_renderer )
    {
      _renderer = createRenderer( );
    }

    //initCapabilites( );
    _surface = _instance->createSurfaceKHR( _window->getWindow( ) );

    // Initializating queues
    auto queueFamilyIndices = _physicalDevice->getQueueFamilyProperties( );
    assert( !queueFamilyIndices.empty( ) );

    _gfxQueueFamilyIdx = uint32_t( -1 );
    _presQueueFamilyIdx = uint32_t( -1 );

    for ( size_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
    {
      VkBool32 presentSupport = _physicalDevice->supportSurfaceKHR(
        i, _surface );  // TODO: i????

      if ( _gfxQueueFamilyIdx == uint32_t( -1 ) &&
        ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics )
        && presentSupport )
      {
        _gfxQueueFamilyIdx = i;
      }
    }

    if ( _gfxQueueFamilyIdx != uint32_t( -1 ) )
    {
      _presQueueFamilyIdx = _gfxQueueFamilyIdx;
    }
    else {
      std::cerr << "No queue with graphics + present; trying separate queues" 
        << std::endl;
      for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
      {
        if ( _gfxQueueFamilyIdx == uint32_t( -1 ) &&
          ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics ) )
        {
          _gfxQueueFamilyIdx = i;
        }

        VkBool32 presentSupport = _physicalDevice->supportSurfaceKHR( i, _surface );
        if ( _presQueueFamilyIdx == uint32_t( -1 ) && presentSupport )
        {
          _presQueueFamilyIdx = i;
        }
      }
    }
    if ( _gfxQueueFamilyIdx == uint32_t( -1 ) )
    {
      std::cerr << "ERROR: No graphics queue family found" << std::endl;
      throw;
    }
    if ( _presQueueFamilyIdx == uint32_t( -1 ) )
    {
      std::cerr << "ERROR: No present queue family found" << std::endl;
      throw;
    }

    auto surfaceFormats = _physicalDevice->getSurfaceFormats( _surface );
    assert( !surfaceFormats.empty( ) );
    uint32_t numFormats = surfaceFormats.size( );

    _colorFormat = vk::Format::eB8G8R8A8Unorm;
    _colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

    VkBool32 validDepthFormat = lava::utils::getSupportedDepthFormat(
      _physicalDevice, _dsFormat );
    assert( validDepthFormat );

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

    _commandPool = _device->createCommandPool(
      { }, //vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
      _gfxQueueFamilyIdx );


    if ( !createDefaultRenderPass( ) )
    {
      return;
    }

    recreateSwapChain( ); // TODO: Move to another zone

    if ( _renderer )
    {
      _renderer->initResources( );
    }
  }

  void VulkanWindow::reset( void )
  {
    _device->waitIdle( );
    if ( _renderer )
    {
      _renderer->releaseResources( );
      _device->waitIdle( );
    }

    if ( _defaultRenderPass )
    {
      _defaultRenderPass.reset( );
      _defaultRenderPass = nullptr;
    }

    if ( _commandPool )
    {
      _commandPool.reset( );
      _commandPool = nullptr;
    }
    
    _surface.reset( );
    _device.reset( );
  }

  VulkanWindow::VulkanWindow( void )
  {
    _swapChainImageSize = glm::ivec2( 500, 500 );
  }

  VulkanWindow::~VulkanWindow( void )
  {
    delete _renderer;
  }

  void VulkanWindow::setVulkanInstance( const std::shared_ptr<Instance> inst )
  {
    this->_instance = inst;
  }

  void VulkanWindow::show( void )
  {
    if ( !_initialized )
    {
      init( );
      _initialized = true;
    }

    while ( _window->isRunning( ) )
    {
      _window->pollEvents( );
      beginFrame( );


      // Very crude method to prevent your GPU from overheating.
      std::this_thread::sleep_for( std::chrono::milliseconds( 16 ) );
    }
  }

  VulkanWindowRenderer * VulkanWindow::createRenderer( void )
  {
    return nullptr;
  }

  void VulkanWindow::frameReady( void )
  {
    // TODO: Check only called by main thread std::this_thread::
    if ( !_framePending )
    {
      throw "framePending() called without calling nextFrame( )";
    }
    _framePending = false;

    endFrame( );
  }

  std::shared_ptr<PhysicalDevice> VulkanWindow::physicalDevice( void )
  {
    return _physicalDevice;
  }

  const vk::PhysicalDeviceProperties* 
    VulkanWindow::physicalDeviceProperties( void ) const
  {
    if ( _physicalDevice )
    {
      return &_physicalDevice->getDeviceProperties( );
    }
    return nullptr;
  }

  std::shared_ptr<Device> VulkanWindow::device( void )
  {
    return _device;
  }

  std::shared_ptr<Queue> VulkanWindow::gfxQueue( void )
  {
    return _gfxQueue;
  }

  std::shared_ptr<CommandPool> VulkanWindow::gfxCommandPool( void )
  {
    return _commandPool;
  }

  std::shared_ptr<RenderPass> VulkanWindow::defaultRenderPass( void )
  {
    return _defaultRenderPass;
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

  std::shared_ptr<CommandBuffer> VulkanWindow::currentCommandBuffer( void ) const
  {
    if ( !_framePending )
    {
      return nullptr;
    }
    return renderPassResources[ _currentFrame ].cmdBuffer;
  }

  std::shared_ptr<Framebuffer> VulkanWindow::currentFramebuffer( void ) const
  {
    if ( !_framePending )
    {
      return nullptr;
    }
    return renderPassResources[ _currentFrame ].framebuffer;
  }

  int VulkanWindow::currentFrame( void ) const
  {
    /*if ( !_framePending )
    {
      return nullptr;
    }*/
    return _currentFrame;
  }

  std::shared_ptr<Image> VulkanWindow::swapChainImage( uint16_t idx ) const
  {
    if ( idx < 0 && idx > 2 )
    {
      return nullptr;
    }
    return renderPassResources[ idx ].image;
  }

  std::shared_ptr<ImageView> VulkanWindow::swapChainImageView( uint16_t idx ) const
  {
    if ( idx < 0 && idx > 2 )
    {
      return nullptr;
    }
    return renderPassResources[ idx ].imageView;
  }

  std::shared_ptr<Image> VulkanWindow::depthStencilImage( void ) const
  {
    return _depthImage;
  }

  std::shared_ptr<ImageView> VulkanWindow::depthStencilImageView( void ) const
  {
    return _depthView;
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
    for ( uint32_t i = 0, 
      l = sizeof( sampleCounts ) / sizeof( sampleCounts[ 0 ] ); 
      i < l; ++i )
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

    for ( uint32_t i = 0, l = sizeof( sampleCounts ) / sizeof( sampleCounts[ 0 ] );
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
  glm::mat4 VulkanWindow::clipCorrectionMatrix( void )
  {
    if ( _clipCorrect == glm::mat4( 1.0f ) )
    {
      _clipCorrect = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.5f,
        0.0f, 0.0f, 0.0f, 1.0f
      );
    }
    return _clipCorrect;
  }
  void VulkanWindow::setWindowTitle( const char*& name )
  {
    setWindowTitle( std::string( name ) );
  }

  void VulkanWindow::setWindowTitle( const std::string& name )
  {
    _window->setWindowTitle( name );
  }

  void VulkanWindow::setDeviceExtensions( const std::vector< std::string >& exts )
  {
    _requestedDeviceExts = exts;
  }
}