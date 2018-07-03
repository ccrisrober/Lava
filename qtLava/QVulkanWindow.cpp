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

#include "QVulkanWindow.h"

#include <QPlatformSurfaceEvent>
#include <QGuiApplication>
#include <QApplication>
#include <QThread>

namespace lava
{
  std::set<std::string> GetSurfaceExtensionsForPlatform( void )
  {
    const auto platform = QGuiApplication::platformName( );

    if ( platform == "xcb" )
    {
      return{
        "VK_KHR_surface",
        "VK_KHR_xcb_surface"
      };
    }
    else if ( platform == "wayland" )
    {
      return{
        "VK_KHR_surface",
        "VK_KHR_wayland_surface"
      };
    }
    else if ( platform == "windows" )
    {
      return{
        "VK_KHR_surface",
        "VK_KHR_win32_surface"
      };
    }
    else if ( platform == "cocoa" )
    {
      return{
        "VK_KHR_surface",
        "VK_MVK_macos_surface"
      };
    }
    throw;
  }

  QtVulkanWindowRenderer* QtVulkanWindow::createRenderer( void )
  {
    return nullptr;
  }

  void QtVulkanWindow::setVkInstance( const vk::Instance& inst )
  {
    _instance = lava::Instance::createFromVkInstance( inst );
    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    assert( _instance->getPhysicalDeviceCount( ) != 0 );
    _physicalDevice = _instance->getPhysicalDevice( 0 );
  }

  QtVulkanWindow::QtVulkanWindow( QWindow* parent )
    : QWindow( parent )
    , _initialized( false )
  {
    setSurfaceType( QSurface::VulkanSurface );

    lava::Engine::CreateInfo ci;
    ci.appInfo = "QtRender";
    ci.enableValidationLayers = true;
    ci.requiredInstanceExtensions = lava::GetSurfaceExtensionsForPlatform( );

    //ci.requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    Engine e( ci );

    inst.setVkInstance( static_cast< VkInstance >( e.GetVkInstance( ) ) );

    if ( !inst.create( ) )
    {
      qFatal( "Failed to create vulkan instance: %d", inst.errorCode( ) );
    }
    setVulkanInstance( &inst );

    _instance = lava::Instance::createFromVkInstance( e.GetVkInstance( ) );

    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    assert( _instance->getPhysicalDeviceCount( ) != 0 );
    _physicalDevice = _instance->getPhysicalDevice( 0 );
  }

  QtVulkanWindow::~QtVulkanWindow( void )
  {
    cleanupVulkan( );
  }
  
  void QtVulkanWindow::beginFrame( void )
  {
    if ( !_dfbFramebuffer || !_dfbFramebuffer->swapchain( ) || _framePending ) return;

    if ( _dfbFramebuffer->extent( ) != swapchainImageSize( ) )
    {
      recreateSwapchain( );
      if ( !_dfbFramebuffer->swapchain( ) )
      {
        return;
      }
    }

    // move on to next swapchain image
    auto res = _dfbFramebuffer->swapchain( )->acquireNextImage( );

    if ( res.result == vk::Result::eErrorOutOfDateKHR )
    {
      qWarning( "Swapchain out of date" );
      // swapchain is out of date (e.g. the window was resized) and
      // must be recreated:
      recreateSwapchain( );
      requestUpdate( );
      return;
    }
    else if ( res.result != vk::Result::eSuccess &&
      res.result != vk::Result::eSuboptimalKHR )
    {
      qWarning( "Swapchain SUBOPTIMAL" );
      // swapchain is not as optimal as it could be, but the platform's
      // presentation engine will still present the image correctly.
      throw std::runtime_error( "Failed to acquire swapchain image" );
    }

    imageIdx = res.value;
    cmds[ imageIdx ] = _cmdPool->allocateCommandBuffer( );

    auto cmd = cmds[ imageIdx ];
    cmd->begin( );

    if ( _frameGrabbing )
    {
      _frameGrabTargetImage = QImage( size( ), QImage::Format_RGBA8888 );
    }

    if ( renderer )
    {
      _framePending = true;
      renderer->nextFrame( );
      // renderer call to frameReady( ) who calls endFrame( )
    }
    else
    {
      std::array<vk::ClearValue, 2 > clearValues;
      std::array<float, 4> ccv = { 1.0f, 1.0f, 1.0f, 1.0f };

      clearValues[ 0 ].color = vk::ClearColorValue( ccv );
      clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

      cmd->beginRenderPass( _dfbFramebuffer->renderPass( ),
        _dfbFramebuffer->framebuffer( imageIdx ),
        vk::Rect2D( { 0, 0 }, swapchainImageSize( ) ), clearValues,
        vk::SubpassContents::eInline );

      cmd->endRenderPass( );

      endFrame( );
    }
  }
  
  void QtVulkanWindow::endFrame( void )
  {
    if ( !isExposed( ) )
    {
      return;
    }
    // RENDER
    {
      try
      {
        auto cmd = cmds[ imageIdx ];

        if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx && !_frameGrabbing )
        {
          // Add swapchain image release to the command buffer that will be
          // submitted to the graphics queue.
          lava::ImageMemoryBarrier presTrans( 
            vk::AccessFlagBits( ), vk::AccessFlagBits::eColorAttachmentWrite, 
            vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::ePresentSrcKHR, 
            _gfxQueueFamilyIdx, _presQueueFamilyIdx, 
            nullptr, // TODO: IMAGE
            vk::ImageSubresourceRange( )
              .setAspectMask( vk::ImageAspectFlagBits::eColor )
              .setLevelCount( 1 )
              .setLayerCount( 1 )
          );

          cmd->pipelineBarrier( vk::PipelineStageFlagBits::eColorAttachmentOutput, 
            vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlagBits( ), 
            { }, { }, presTrans );
        }

        // When grabbing a frame, add a readback at the end and skip presenting.
        if ( _frameGrabbing )
        {
          addReadback( );
        }

        cmd->end( );

        /*vk::Result res = */_gfxQueue->submit( SubmitInfo{
          _dfbFramebuffer->swapchain( )->getPresentCompleteSemaphores( )[ imageIdx ],
          { vk::PipelineStageFlagBits::eColorAttachmentOutput },
          cmd,
          _renderComplete
        } );
      }
      catch ( std::exception e )
      {
        std::cerr << e.what( ) << std::endl;
      }
    }

    // block and then bail out when grabbing
    if ( _frameGrabbing )
    {
      finishBlockingReadback( );
      _frameGrabbing = false;
      // Leave frame.imageAcquired set to true.
      // Don't change currentFrame.
      //emit frameGrabbed( frameGrabTargetImage );
      return;
    }

    if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx )
    {
      /*// Submit the swapchain image acquire to the present queue.
      submitInfo.pWaitSemaphores = &frame.drawSem;
      submitInfo.pSignalSemaphores = &frame.presTransSem;
      submitInfo.pCommandBuffers = &image.presTransCmdBuf; // must be USAGE_SIMULTANEOUS
      err = devFuncs->vkQueueSubmit( presQueue, 1, &submitInfo, VK_NULL_HANDLE );
      if ( err != vk::Result::eSuccess )
      {
        if ( !checkDeviceLost( err ) )
        {
          qWarning( "QVulkanWindow: Failed to submit to present queue: %d", err );
        }
        return;
      }*/
    }





    vk::Result presentResult = vk::Result::eSuccess;
    try
    {
      auto waitSemaphores = { _renderComplete };
      auto swapchains = { _dfbFramebuffer->swapchain( ) };
      std::vector<uint32_t> imageIndices = { imageIdx };
      vk::Queue _queue = *_presQueue;
      {
        std::vector<vk::Semaphore> waitSemaphoreData;
        waitSemaphoreData.reserve( waitSemaphores.size( ) );
        for ( auto const& s : waitSemaphores )
        {
          waitSemaphoreData.push_back( *s );
        }

        std::vector<vk::SwapchainKHR> swapchainData;
        swapchainData.reserve( swapchains.size( ) );
        for ( auto const& s : swapchains )
        {
          swapchainData.push_back( static_cast< vk::SwapchainKHR >( *s ) );
        }

        presentResult = _queue.presentKHR( vk::PresentInfoKHR(
          waitSemaphoreData.size( ), waitSemaphoreData.data( ),
          swapchainData.size( ), swapchainData.data( ),
          imageIndices.data( ) ) );
        //return results;
      }
      //_presQueue->present( { signalSemaphore } )
    }
    catch ( vk::OutOfDateKHRError )
    {
      recreateSwapchain( );
      presentResult = vk::Result::eSuccess;
    }
    catch ( std::exception e )
    {
      std::cout << e.what( ) << std::endl;
    }
    if ( presentResult == vk::Result::eSuboptimalKHR )
    {
      recreateSwapchain( );
    }
    else if ( presentResult != vk::Result::eSuccess )
    {
      throw "Failed to present swap chain image.";
    }

    vulkanInstance( )->presentQueued( this );
    _presQueue->waitIdle( ); // TODO: Neccesary??
    
    if ( _continuousAnimation )
    {
      requestUpdate( );
    }
  }
  
  void QtVulkanWindow::frameReady( void )
  {
    // TODO: Check only called by main thread std::this_thread::
    if ( QThread::currentThread( ) != QApplication::instance( )->thread( ) )
    {
      std::cerr << "You only can called this in main thread" << std::endl;
      return;
    }
    if ( !_framePending )
    {
      throw "framePending() called without calling nextFrame( )";
    }
    _framePending = false;

    endFrame( );
  }

  QImage QtVulkanWindow::grab( void )
  {
    if ( !_dfbFramebuffer->swapchain( ) )
    {
      qWarning( "Attempted to call grab() without swapchain" );
      return QImage( );
    }
    if ( _framePending )
    {
      qWarning( "Attempted to call grab() while a frame is still pending" );
      return QImage( );
    }
    if ( !supportGrab( ) )
    {
      qWarning( "Attempted to call grab() with swapchain that doesn't support usage as transfer source" );
      return QImage( );
    }

    _frameGrabbing = true;
    beginFrame( );

    return _frameGrabTargetImage;
  }

  static struct {
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

  void QtVulkanWindow::setSampleCountFlagBits( int sampleCount_ )
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

  std::vector<int> QtVulkanWindow::supportedSampleCounts( void )
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
  
  bool QtVulkanWindow::event( QEvent * ev )
  {
    switch ( ev->type( ) )
    {
    case QEvent::UpdateRequest:
      beginFrame( );
      break;
    case QEvent::PlatformSurface:
    {
      auto surfEv = ( ( QPlatformSurfaceEvent * ) ( ev ) )->surfaceEventType( );
      if ( surfEv == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed )
      {
        if ( renderer )
        {
          renderer->releaseSwapchainResources( );
        }
        cleanupVulkan( );
      }
      break;
    }
    default:
      break;
    }

    return QWindow::event( ev );
  }
  
  void QtVulkanWindow::exposeEvent( QExposeEvent * ev )
  {
    if ( isExposed( ) )
    {
      if ( !_initialized )
      {
        initVulkan( );
        if ( renderer )
        {
          renderer->initSwapchainResources( );
        }
      }
      requestUpdate( );
    }

    QWindow::exposeEvent( ev );
  }
  
  void QtVulkanWindow::resizeEvent( QResizeEvent * ev )
  {
  }

  bool QtVulkanWindow::checkDeviceLost( vk::Result err )
  {
    if ( err == vk::Result::eErrorDeviceLost )
    {
      qWarning( "Device lost" );
      /*if ( renderer )
      {
        renderer->logicalDeviceLost( );
      }
      qWarning( "Releasing all resources due to device lost" );
      releaseSwapchain( );
      reset( );
      qWarning( "Restarting" );
      ensureStarted( );*/
      return true;
    }
    return false;
  }

  void QtVulkanWindow::addReadback( void )
  {
    /*std::shared_ptr< lava::Image > frameGrabImage = 
      _device->createImage( vk::ImageCreateFlagBits( ), vk::ImageType::e2D, 
        vk::Format::eR8G8B8A8Unorm, 
        vk::Extent3D( 
          _frameGrabTargetImage.width( ), _frameGrabTargetImage.height( ), 1
        ), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear, 
        vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode( ), { }, 
        vk::ImageLayout::ePreinitialized, nullptr );

    auto cmd = cmds[ imageIdx ];*/
  }

  void QtVulkanWindow::finishBlockingReadback( void )
  {
    // TODO
  }

  void QtVulkanWindow::getSurfaceFormats( void )
  {
    auto surfaceFormats = _physicalDevice->getSurfaceFormats( _surface );
    assert( !surfaceFormats.empty( ) );
    uint32_t numFormats = surfaceFormats.size( );

    bool gamma = false;
    // If there is no preferred format, use standard RGBA
    if ( ( numFormats == 1 )
      && ( surfaceFormats[ 0 ].format == vk::Format::eUndefined ) )
    {
      _surfaceFormat.format = gamma ? vk::Format::eR8G8B8A8Srgb : vk::Format::eB8G8R8A8Unorm;

      _surfaceFormat.colorSpace = surfaceFormats[ 0 ].colorSpace;
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

      std::vector<vk::Format> wantedFormats = gamma ? wantedFormatsSRGB : wantedFormatsUNORM;

      for ( const auto& wantedFormat : wantedFormats )
      {
        for ( const auto& surfFormat : surfaceFormats )
        {
          if ( surfFormat.format == wantedFormat )
          {
            _surfaceFormat.format = surfFormat.format;
            _surfaceFormat.colorSpace = surfFormat.colorSpace;

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
        _surfaceFormat.format = surfaceFormats[ 0 ].format;
        _surfaceFormat.colorSpace = surfaceFormats[ 0 ].colorSpace;

        if ( gamma )
        {
          throw new std::runtime_error( R"(Cannot find a valid sRGB format for a render window surface, falling back to a default format.)" );
        }
      }
    }
  }

  void QtVulkanWindow::createQueues( void )
  {
    // Search for a graphics queue and a present queue in the array of 
    //    queue families, try to find one that supports both
    std::vector<vk::QueueFamilyProperties> queueFamilyIndices =
      _physicalDevice->getQueueFamilyProperties( );
    assert( !queueFamilyIndices.empty( ) );

    _gfxQueueFamilyIdx = uint32_t( -1 );
    _presQueueFamilyIdx = uint32_t( -1 );

    VkBool32 presentSupport = VK_FALSE;
    for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
    {
      presentSupport = _physicalDevice->supportSurfaceKHR( i, _surface );

      printf( "queue family %d: flags=0x%x count=%d supportsPresent=%d\n",
        i,
        static_cast< VkQueueFlags >( queueFamilyIndices[ i ].queueFlags ),
        queueFamilyIndices[ i ].queueCount,
        presentSupport == VK_TRUE ? 1 : 0
      );

      if ( _gfxQueueFamilyIdx == uint32_t( -1 ) &&
        ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics )
        && presentSupport )
      {
        _gfxQueueFamilyIdx = i;
        break;
      }
    }

    if ( _gfxQueueFamilyIdx != uint32_t( -1 ) )
    {
      _presQueueFamilyIdx = _gfxQueueFamilyIdx;
    }
    else
    {
      std::cerr << "No queue with graphics + present; trying separate queues"
        << std::endl;
      for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
      {
        if ( _gfxQueueFamilyIdx == uint32_t( -1 ) &&
          ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics ) )
        {
          _gfxQueueFamilyIdx = i;
        }

        presentSupport = _physicalDevice->supportSurfaceKHR( i, _surface );
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
  }
  
  bool QtVulkanWindow::setupPipelineCache( void )
  {
    _pipelineCache = nullptr;
    return true;
  }

  void QtVulkanWindow::initVulkan( void )
  {
    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    if ( !renderer )
    {
      renderer = createRenderer( );
    }

    if ( !_physicalDevice )
    {
      LAVA_RUNTIME_ERROR( "Failed to find a device with presentation support" );
    }

    _surface = std::make_shared< lava::Surface >( _instance, 
      vk::SurfaceKHR( QVulkanInstance::surfaceForWindow( this ) ), false );

    getSurfaceFormats( );

    createQueues( );

    VkBool32 validDepthFormat = lava::utils::getSupportedDepthFormat(
      _physicalDevice, _dsFormat );
    assert( validDepthFormat );

    _dfbFramebuffer = new DefaultFramebuffer( _device, _surface, 
      swapchainImageSize( ), _dsFormat, sampleCount );

    _cmdPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits( ),
      //vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
      _gfxQueueFamilyIdx );

    size_t numImages = _dfbFramebuffer->swapchain( )->count( );
    cmds.reserve( numImages );
    for ( size_t i = 0; i < numImages; ++i )
    {
      cmds.push_back( _cmdPool->allocateCommandBuffer( ) );
    }

    setupPipelineCache( );

    if ( renderer )
    {
      renderer->initResources( );
    }

    _renderComplete = _device->createSemaphore( );

    _initialized = true;
  }
  
  void QtVulkanWindow::cleanupVulkan( void )
  {
    if ( !_initialized ) return;

    _device->waitIdle( );

    if ( renderer )
    {
      renderer->releaseResources( );
      _device->waitIdle( );

      delete renderer;

      renderer = nullptr;
    }

    cmds.clear( );

    delete _dfbFramebuffer;
    _dfbFramebuffer = nullptr;

    if ( _renderComplete )
    {
      _renderComplete.reset( );
      _renderComplete = nullptr;
    }

    _initialized = false;
  }
  
  void QtVulkanWindow::recreateSwapchain( void )
  {
    _dfbFramebuffer->recreate( );
  }
}