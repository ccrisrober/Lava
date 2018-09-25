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

#include <iostream>

#include <QLoggingCategory>

namespace pompeii
{
  namespace qt
  {
    VulkanWindow::Engine::Engine( const CreateInfo &info )
      : info( info )
    {
      createInstance( );
    }

    VulkanWindow::Engine::~Engine( void )
    {

      //instance.destroy( nullptr );
    }

    void VulkanWindow::Engine::createInstance( void )
    {
      vk::ApplicationInfo appInfo(
        info.appInfo.c_str( ),
        VK_MAKE_VERSION( 1, 0, 0 ),
        "LavaEngine",
        VK_MAKE_VERSION( 1, 0, 0 ),
        VK_API_VERSION_1_0
      );

#ifndef NDEBUG
      QLoggingCategory::setFilterRules( QStringLiteral( "qt.vulkan=true" ) );
#endif

    vk::InstanceCreateInfo ici;
    ici.setPApplicationInfo( &appInfo );

    // Extensions

    auto extensionsdExts = vk::enumerateInstanceExtensionProperties( );
    std::cout << "Available Extensions:" << std::endl;
    for ( const auto &extension : extensionsdExts )
    {
      std::cout << "\t" << extension.extensionName << std::endl;
    }

    std::vector<const char *> requiredExts;
    {
      for ( const auto &extension : info.requiredInstanceExtensions )
      {
        requiredExts.push_back( extension.c_str( ) );
      }

      if ( info.enableValidationLayers )
      {
        requiredExts.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
      }
    }

    ici.setEnabledExtensionCount( static_cast< uint32_t >( requiredExts.size( ) ) );
    ici.setPpEnabledExtensionNames( requiredExts.data( ) );

    std::vector<const char* > layers;
    {
      layers = {
  #ifndef NDEBUG
    #ifndef Q_OS_ANDROID
          "VK_LAYER_LUNARG_standard_validation"
    #else
          "VK_LAYER_GOOGLE_threading",
          "VK_LAYER_LUNARG_parameter_validation",
          "VK_LAYER_LUNARG_object_tracker",
          "VK_LAYER_LUNARG_core_validation",
          "VK_LAYER_LUNARG_swapchain",
          "VK_LAYER_GOOGLE_unique_objects"
    #endif
  #endif
        };
        if ( info.enableRenderdoc )
        {
          // TODO
        }
      }
      ici.setEnabledLayerCount( static_cast< uint32_t >( layers.size( ) ) );
      ici.setPpEnabledLayerNames( layers.data( ) );

      instance = vk::createInstance( ici );
    }

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

    VulkanWindowRenderer* VulkanWindow::createRenderer( void )
    {
      return nullptr;
    }

    void VulkanWindow::setVkInstance( const vk::Instance& inst )
    {
      _instance = pompeii::Instance::createFromVkInstance( inst );
      if ( !_instance )
      {
        throw "Instance don't exist";
      }

      assert( _instance->getPhysicalDeviceCount( ) != 0 );
      _physicalDevice = _instance->getPhysicalDevice( 0 );
    }

    VulkanWindow::VulkanWindow( QWindow* parent )
      : QWindow( parent )
      , _initialized( false )
    {
      setSurfaceType( QSurface::VulkanSurface );

      pompeii::qt::VulkanWindow::Engine::CreateInfo ci;
      ci.appInfo = "QtRender";
      ci.enableValidationLayers = true;
      ci.requiredInstanceExtensions = pompeii::qt::GetSurfaceExtensionsForPlatform( );

      //ci.requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
      Engine e( ci );

      inst.setVkInstance( static_cast< VkInstance >( e.GetVkInstance( ) ) );

      if ( !inst.create( ) )
      {
        qFatal( "Failed to create vulkan instance: %d", inst.errorCode( ) );
      }
      setVulkanInstance( &inst );

      _instance = pompeii::Instance::createFromVkInstance( e.GetVkInstance( ) );

      if ( !_instance )
      {
        throw "Instance don't exist";
      }

      assert( _instance->getPhysicalDeviceCount( ) != 0 );
      _physicalDevice = _instance->getPhysicalDevice( 0 );
    }

    VulkanWindow::~VulkanWindow( void )
    {
      cleanupVulkan( );
    }
    
    void VulkanWindow::beginFrame( void )
    {
      if ( !_swapchain || _framePending ) return;

      if ( _swapchain->extent( ) != swapchainImageSize( ) )
      {
        recreateSwapchain( );
        if ( !_swapchain )
        {
          return;
        }
      }

      // move on to next swapchain image
      auto res = _swapchain->acquireNextImage( );

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

        cmd->beginRenderPass( _renderPass,
          _framebuffers.at( imageIdx ),
          vk::Rect2D( { 0, 0 }, swapchainImageSize( ) ), clearValues,
          vk::SubpassContents::eInline );

        cmd->endRenderPass( );

        endFrame( );
      }
    }
    
    void VulkanWindow::endFrame( void )
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
            pompeii::ImageMemoryBarrier presTrans( 
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
            _swapchain->getPresentCompleteSemaphores( )[ imageIdx ],
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
        auto swapchains = { _swapchain };
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

    void VulkanWindow::setupDepthStencilTarget( void )
    {
      // depth/stencil buffer
      // assert that a depth and/or stencil format is requested
      vk::FormatProperties formatProps =
        _device->getPhysicalDevice( )->getFormatProperties( _dsFormat );
      assert( ( formatProps.linearTilingFeatures &
        vk::FormatFeatureFlagBits::eDepthStencilAttachment ) ||
        ( formatProps.optimalTilingFeatures &
          vk::FormatFeatureFlagBits::eDepthStencilAttachment ) );

      vk::ImageTiling tiling = ( formatProps.optimalTilingFeatures
        & vk::FormatFeatureFlagBits::eDepthStencilAttachment )
        ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear;

      auto extent = _swapchain->extent( );

      _depthImage = _device->createImage(
        vk::ImageCreateFlagBits( ), vk::ImageType::e2D, _dsFormat,
        vk::Extent3D( extent.width, extent.height, 1 ), 1, 1,
        sampleCount, tiling,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive, { },
        vk::ImageLayout::eUndefined, { } );

      // determine ImageAspect based on format
      vk::ImageAspectFlags aspectFlags;
      if ( _dsFormat != vk::Format::eS8Uint )
      {
        aspectFlags |= vk::ImageAspectFlagBits::eDepth;
      }

      // add eStencil if image contains stencil
      static std::initializer_list<vk::Format> const stencilFormats{
        vk::Format::eD16UnormS8Uint, vk::Format::eD24UnormS8Uint,
        vk::Format::eD32SfloatS8Uint, vk::Format::eS8Uint };
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
    }
    
    void VulkanWindow::frameReady( void )
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

    QImage VulkanWindow::grab( void )
    {
      if ( !_swapchain )
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
    
    bool VulkanWindow::event( QEvent * ev )
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
    
    void VulkanWindow::exposeEvent( QExposeEvent * ev )
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
    
    void VulkanWindow::resizeEvent( QResizeEvent * ev )
    {
    }

    bool VulkanWindow::checkDeviceLost( vk::Result err )
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

    void VulkanWindow::addReadback( void )
    {
      frameGrabImage = 
        _device->createImage( vk::ImageCreateFlagBits( ), vk::ImageType::e2D, 
          vk::Format::eR8G8B8A8Unorm, 
          vk::Extent3D( 
            _frameGrabTargetImage.width( ), _frameGrabTargetImage.height( ), 1
          ), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear, 
          vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode( ), { }, 
          vk::ImageLayout::ePreinitialized,
          vk::MemoryPropertyFlagBits::eHostVisible );

      auto cmd = cmds[ imageIdx ];

      auto image = _swapchain->images( )[ imageIdx ];

      vk::ImageSubresourceRange isr;
      isr.aspectMask = vk::ImageAspectFlagBits::eColor;
      isr.levelCount = isr.layerCount = 1;
      ImageMemoryBarrier imr( 
        vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eTransferRead, 
        vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferSrcOptimal, 
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, 
        image, isr
      );

      cmd->pipelineBarrier( 
        vk::PipelineStageFlagBits::eColorAttachmentOutput, 
        vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, imr );

      imr.oldLayout = vk::ImageLayout::ePreinitialized;
      imr.newLayout = vk::ImageLayout::eTransferDstOptimal;
      imr.srcAccessMask = { };
      imr.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

      cmd->pipelineBarrier(
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, imr );

      vk::ImageCopy copyInfo;
      copyInfo.srcSubresource.aspectMask = copyInfo.dstSubresource.aspectMask = 
        vk::ImageAspectFlagBits::eColor;
      copyInfo.srcSubresource.layerCount = copyInfo.dstSubresource.layerCount = 1;
      copyInfo.extent.width = _frameGrabTargetImage.width( );
      copyInfo.extent.height = _frameGrabTargetImage.height( );
      copyInfo.extent.depth = 1;

      cmd->copyImage( image, vk::ImageLayout::eTransferSrcOptimal, 
        frameGrabImage, vk::ImageLayout::eTransferDstOptimal, copyInfo
      );


      imr.oldLayout = vk::ImageLayout::eTransferDstOptimal;
      imr.newLayout = vk::ImageLayout::eGeneral;
      imr.srcAccessMask = vk::AccessFlagBits::eHostRead;

      cmd->pipelineBarrier(
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, imr );
    }

    void VulkanWindow::finishBlockingReadback( void )
    {
      // TODO
      auto image = _swapchain->images( )[ imageIdx ];

      // Block until the current frame is done. Normally this wait would only be
      // done in current + concurrentFrameCount().
      //pompeii::Fence::waitForFences( { resources.cmdFence }, VK_TRUE, maxUint );


      vk::ImageSubresource subRes = { vk::ImageAspectFlagBits::eColor, 0, 0 };
      vk::Device d = static_cast< vk::Device >( *_device );
      vk::SubresourceLayout layout = d.getImageSubresourceLayout( 
        *frameGrabImage, subRes );

//      uchar* p;

      frameGrabImage = nullptr;
    }

    void VulkanWindow::getSurfaceFormats( void )
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

    void VulkanWindow::createQueues( void )
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
    
    void VulkanWindow::setupRenderPass( void )
    {
      // Creating renderpass
      std::vector< vk::AttachmentDescription > attDesc =
      {
        vk::AttachmentDescription( // attachment 0 (color render target)
          vk::AttachmentDescriptionFlagBits( ),
          _swapchain->colorFormat( ), vk::SampleCountFlagBits::e1,
          vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, // color
          vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, // stencil
          vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
        ),
        vk::AttachmentDescription( // attachment 1 (depth render target)
          vk::AttachmentDescriptionFlagBits( ),
          _dsFormat, sampleCount,
          vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, // depth
          vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, // stencil
          vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
        )
      };

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

      _renderPass = _device->createRenderPass( attDesc, subPassDesc, { } );
    }

    void VulkanWindow::setupFramebuffer( void )
    {
      // TODO: Unnecesary ? _framebuffers.clear( );
      vk::Extent2D extent = _swapchain->extent( );

      _framebuffers.reserve( _swapchain->imageViews( ).size( ) );
      for ( size_t i = 0, l = _swapchain->imageViews( ).size( ); i < l; ++i )
      {
        _framebuffers.push_back( _device->createFramebuffer( _renderPass,
        { _swapchain->imageViews( )[ i ], _depthView },
          extent, 1 ) );
      }

      std::cout << "Framebuffer Swapchain OK" << std::endl;
    }

    bool VulkanWindow::setupPipelineCache( void )
    {
      _pipelineCache = nullptr;
      return true;
    }

    void VulkanWindow::initVulkan( void )
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
        POMPEII_RUNTIME_ERROR( "Failed to find a device with presentation support" );
      }

      _surface = std::make_shared< pompeii::Surface >( _instance, 
        vk::SurfaceKHR( QVulkanInstance::surfaceForWindow( this ) ), false );

      getSurfaceFormats( );

      createQueues( );

      VkBool32 validDepthFormat = utils::getSupportedDepthFormat(
        _physicalDevice, _dsFormat );
      assert( validDepthFormat );

      _swapchain = std::make_shared< Swapchain >( _device, 
        _surface, swapchainImageSize( ) );

      setupDepthStencilTarget( );
      setupRenderPass( );
      setupFramebuffer( );

      _cmdPool = _device->createCommandPool(
        //vk::CommandPoolCreateFlagBits( ),
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        _gfxQueueFamilyIdx );

      size_t numImages = _swapchain->count( );
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
    
    void VulkanWindow::cleanupVulkan( void )
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

      _swapchain.reset( );
      _swapchain = nullptr;

      if ( _renderComplete )
      {
        _renderComplete.reset( );
        _renderComplete = nullptr;
      }

      _initialized = false;
    }
    
    void VulkanWindow::recreateSwapchain( void )
    {
      // recreateSwapchain( )
      {
        _swapchain->recreate( );
      }

      // cleanupFramebuffers( )
      {
        for ( auto& fb : _framebuffers )
        {
          fb.reset( );
        }
        _framebuffers.clear( );
      }

      // cleanupDepth
      {
        _depthView.reset( );
        _depthImage.reset( );
      }

      setupDepthStencilTarget( );
      setupFramebuffer( );

      if( renderer )
      {
        renderer->initSwapchainResources( );
      }
    }
  }
}