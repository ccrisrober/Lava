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

#ifndef __QTLAVA_VULKANWINDOW__
#define __QTLAVA_VULKANWINDOW__

#include <lava/lava.h>
#include <qtLava/api.h>

#include <QWindow>
#include <QtGui/QVulkanWindow>

#include <algorithm>

#include "Engine.h"

#define CLEAR_ARRAY(arr) for( auto&a: arr ) a.reset( ); arr.clear( );

namespace lava
{
  QTLAVA_API
  std::set<std::string> GetSurfaceExtensionsForPlatform( void );

  class QtVulkanWindowRenderer
  {
  public:
    QTLAVA_API
    virtual ~QtVulkanWindowRenderer( void ) { }

    /**
    * Method called when creating renderer's resources
    */
    QTLAVA_API
    virtual void initResources( void ) { }
    QTLAVA_API
    virtual void initSwapchainResources( void ) { }
    QTLAVA_API
    virtual void releaseSwapchainResources( void ) { }
    /**
    * Method called when renderer's resources must be released
    */
    QTLAVA_API
    virtual void releaseResources( void ) { }
    /**
    * Method called when the draw calls for the next frame are to be added
    *   to the command buffer
    */
    QTLAVA_API
    virtual void nextFrame( void ) = 0;

    //virtual void physicalDeviceLost( void );
    //virtual void logicalDeviceLost( void );
  };

  class DefaultFramebuffer
  {
  protected:
    void createFramebuffers( void )
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

    void createRenderPasses( void )
    {
      const bool msaa = sampleCount > vk::SampleCountFlagBits::e1;

      // Creating renderpass
      {
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
            depthFormat, sampleCount,
            vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, // depth
            vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, // stencil
            vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
          )
        };

        if ( msaa )
        {
          attDesc.push_back( vk::AttachmentDescription( // attachment 2 (msaa render target)
            vk::AttachmentDescriptionFlagBits( ),
            _swapchain->colorFormat( ), sampleCount,
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
      }
    }

    void cleanupFramebuffers( void )
    {
      for ( auto& fb : _framebuffers )
      {
        fb.reset( );
      }
      _framebuffers.clear( );
    }

    void cleanupRenderPasses( )
    {
      _renderPass.reset( );
    }
  public:
    QTLAVA_API
    DefaultFramebuffer( const std::shared_ptr< Device>& dev,
      const std::shared_ptr< lava::Surface>& surface,
      vk::Extent2D extent, vk::Format depthFormat_, 
      vk::SampleCountFlagBits sampleCount_)
      : _device( dev )
      , depthFormat( depthFormat_ )
      , sampleCount( sampleCount_ )
    {
      _swapchain = std::make_shared< Swapchain >( _device, surface, extent );

      // depth/stencil buffer
      // assert that a depth and/or stencil format is requested
      vk::FormatProperties formatProps =
        _device->getPhysicalDevice( )->getFormatProperties( depthFormat );
      assert( ( formatProps.linearTilingFeatures &
        vk::FormatFeatureFlagBits::eDepthStencilAttachment ) ||
        ( formatProps.optimalTilingFeatures &
          vk::FormatFeatureFlagBits::eDepthStencilAttachment ) );

      vk::ImageTiling tiling = ( formatProps.optimalTilingFeatures
        & vk::FormatFeatureFlagBits::eDepthStencilAttachment )
        ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear;

      _depthImage = _device->createImage(
        vk::ImageCreateFlagBits( ), vk::ImageType::e2D, depthFormat,
        vk::Extent3D( extent.width, extent.height, 1 ), 1, 1,
        sampleCount, tiling,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive, { },
        vk::ImageLayout::eUndefined, { } );

      // determine ImageAspect based on format
      vk::ImageAspectFlags aspectFlags;
      if ( depthFormat != vk::Format::eS8Uint )
      {
        aspectFlags |= vk::ImageAspectFlagBits::eDepth;
      }

      // add eStencil if image contains stencil
      static std::initializer_list<vk::Format> const stencilFormats{
        vk::Format::eD16UnormS8Uint, vk::Format::eD24UnormS8Uint,
        vk::Format::eD32SfloatS8Uint, vk::Format::eS8Uint };
      if ( std::find( stencilFormats.begin( ), stencilFormats.end( ),
        depthFormat ) != stencilFormats.end( ) )
      {
        aspectFlags |= vk::ImageAspectFlagBits::eStencil;
      }

      _depthView = _depthImage->createImageView(
        vk::ImageViewType::e2D, depthFormat,
        vk::ComponentMapping(
          vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA ),
        vk::ImageSubresourceRange( aspectFlags, 0, 1, 0, 1 ) );

      createRenderPasses( );
      createFramebuffers( );
    }
    
    QTLAVA_API
    ~DefaultFramebuffer( void )
    {
      cleanupFramebuffers( );
      cleanupRenderPasses( );
      _depthView.reset( );
      _depthImage.reset( );
      _swapchain.reset( );
    }
    
    QTLAVA_API
    const vk::Extent2D& extent( void ) const
    {
      return _swapchain->extent( );
    }
    
    QTLAVA_API
    const std::shared_ptr< Swapchain > swapchain( void ) const
    {
      return _swapchain;
    }
    
    QTLAVA_API
    const std::shared_ptr< RenderPass > renderPass( void ) const
    {
      return _renderPass;
    }
    
    QTLAVA_API
    const std::shared_ptr< Framebuffer > framebuffer( short idx ) const
    {
      return _framebuffers.at( idx );
    }
    
    /*QTLAVA_API
    const std::shared_ptr< Framebuffer > framebuffer( void ) const
    {
      static short currentIdx = 0;
      auto fbo = _framebuffers.at( currentIdx );
      ++currentIdx;
      currentIdx = currentIdx % _framebuffers.size( );
      return fbo;
    }*/
    
    QTLAVA_API
    void recreate( void )
    {
      // TODO: RECREATE DEPTH!!

      this->_swapchain->recreate( );

      auto extent = _swapchain->extent( );

      cleanupFramebuffers( );
      cleanupRenderPasses( );
      _depthView.reset( );
      _depthImage.reset( );

      // depth/stencil buffer
      // assert that a depth and/or stencil format is requested
      vk::FormatProperties formatProps =
        _device->getPhysicalDevice( )->getFormatProperties( depthFormat );
      assert( ( formatProps.linearTilingFeatures &
        vk::FormatFeatureFlagBits::eDepthStencilAttachment ) ||
        ( formatProps.optimalTilingFeatures &
          vk::FormatFeatureFlagBits::eDepthStencilAttachment ) );

      vk::ImageTiling tiling = ( formatProps.optimalTilingFeatures
        & vk::FormatFeatureFlagBits::eDepthStencilAttachment )
        ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear;

      _depthImage = _device->createImage(
        vk::ImageCreateFlagBits( ), vk::ImageType::e2D, depthFormat,
        vk::Extent3D( extent.width, extent.height, 1 ), 1, 1,
        sampleCount, tiling,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive, { },
        vk::ImageLayout::eUndefined, { } );

      // determine ImageAspect based on format
      vk::ImageAspectFlags aspectFlags;
      if ( depthFormat != vk::Format::eS8Uint )
      {
        aspectFlags |= vk::ImageAspectFlagBits::eDepth;
      }

      // add eStencil if image contains stencil
      static std::initializer_list<vk::Format> const stencilFormats{
        vk::Format::eD16UnormS8Uint, vk::Format::eD24UnormS8Uint,
        vk::Format::eD32SfloatS8Uint, vk::Format::eS8Uint };
      if ( std::find( stencilFormats.begin( ), stencilFormats.end( ),
        depthFormat ) != stencilFormats.end( ) )
      {
        aspectFlags |= vk::ImageAspectFlagBits::eStencil;
      }

      _depthView = _depthImage->createImageView(
        vk::ImageViewType::e2D, depthFormat,
        vk::ComponentMapping(
          vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA ),
        vk::ImageSubresourceRange( aspectFlags, 0, 1, 0, 1 ) );

      createRenderPasses( );
      createFramebuffers( );
    }
  protected:
    std::shared_ptr < Device > _device;
    std::shared_ptr< Swapchain > _swapchain;
    std::vector<std::shared_ptr<Framebuffer>> _framebuffers;
    std::shared_ptr< Image > _depthImage;
    std::shared_ptr< ImageView > _depthView;
    std::shared_ptr< RenderPass > _renderPass;
    vk::Format depthFormat;
    vk::SampleCountFlagBits sampleCount;
  };

  
  class QtVulkanWindow : public QWindow
  {
    //Q_OBJECT
    /*enum Status
    {
      StatusUninitialized,
      StatusFail,
      StatusFailRetry,
      StatusDeviceReady,
      StatusReady
    };
    Status status = StatusUninitialized; TODO: UNUSED*/
  protected:
    QTLAVA_API
    virtual bool setupPipelineCache( void );
  private:
    void initVulkan( void );
    void cleanupVulkan( void );
    void recreateSwapchain( void );
    bool _initialized;

    bool _continuousAnimation = false;
    
    std::shared_ptr< lava::Instance > _instance = nullptr;
    std::shared_ptr< lava::PhysicalDevice > _physicalDevice = nullptr;
    std::shared_ptr< lava::Device > _device = nullptr;
    
    std::shared_ptr< lava::Surface > _surface;
    //vk::SurfaceKHR _surface = nullptr;


    vk::SurfaceFormatKHR _surfaceFormat;
    vk::Format _dsFormat;

    uint32_t imageIdx;

    std::vector<std::string > _requestedDeviceExts;
    uint32_t _gfxQueueFamilyIdx;
    uint32_t _presQueueFamilyIdx;
    std::shared_ptr< lava::Queue > _gfxQueue;
    std::shared_ptr< lava::Queue > _presQueue;

    std::vector<std::shared_ptr< lava::CommandBuffer > > cmds;

    DefaultFramebuffer* _dfbFramebuffer = nullptr;

    std::shared_ptr< lava::CommandPool > _cmdPool;

    bool _framePending = false;
    bool _frameGrabbing = false;
    QImage _frameGrabTargetImage;
    std::shared_ptr< lava::Image > frameGrabImage = nullptr;
  protected:
    std::shared_ptr< PipelineCache > _pipelineCache;
  private:
    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

    std::shared_ptr< lava::Semaphore > _renderComplete = nullptr;
    QtVulkanWindowRenderer* renderer = nullptr;
  protected:
    virtual QtVulkanWindowRenderer* createRenderer( void );
  public:
    QTLAVA_API
    virtual void setVkInstance( const vk::Instance& instance );
    QTLAVA_API
    explicit QtVulkanWindow( QWindow* parent = 0 );
    QTLAVA_API
    QtVulkanWindow( const QtVulkanWindow& ) = delete;
    QTLAVA_API
    QtVulkanWindow( QtVulkanWindow&& ) = delete;
    QTLAVA_API
    virtual ~QtVulkanWindow( void );

    QTLAVA_API
    QtVulkanWindow& operator=( const QtVulkanWindow& ) = delete;
    QTLAVA_API
    QtVulkanWindow& operator=( QtVulkanWindow&& ) = delete;

    QTLAVA_API
    const std::shared_ptr< PipelineCache > pipelineCache( void )
    {
      return _pipelineCache;
    }
  protected:
    //QTLAVA_API
    void beginFrame( void );
    //QTLAVA_API
    void endFrame( void );
  public:
    QTLAVA_API
    void frameReady( void );

    QTLAVA_API
    QImage grab( void );

    QTLAVA_API
    bool supportGrab( void ) const
    {
      return //_dfbFramebuffer == nullptr ? false : 
        _dfbFramebuffer->swapchain( )->swapchainSupportsReadBack( );
    }

    QTLAVA_API
    vk::Extent2D swapchainImageSize( void ) const
    {
      QSize _size = size( );
      return vk::Extent2D(
        _size.width( ),
        _size.height( )
      );
    }

    QTLAVA_API
    vk::SampleCountFlagBits sampleCountFlagBits( void ) const
    {
      return sampleCount;
    }
    QTLAVA_API
    void setSampleCountFlagBits( int sampleCount );
    QTLAVA_API
    std::vector< int > supportedSampleCounts( void );

    const bool isContinuosRendering( void ) const
    {
      return _continuousAnimation;
    }
    QTLAVA_API
    void setContinuousRendering( bool enabled )
    {
      _continuousAnimation = enabled;
      if ( _initialized )
      {
        requestUpdate( );
      }
    }

    QTLAVA_API
    std::shared_ptr<PhysicalDevice> physicalDevice( void ) const
    {
      return _physicalDevice;
    }
    QTLAVA_API
    const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const
    {
      return physicalDevice( )->getDeviceProperties( );
    }
    QTLAVA_API
    std::shared_ptr<Device> device( void ) const
    {
      return _device;
    }
    QTLAVA_API
    std::shared_ptr<Queue> gfxQueue( void ) const
    {
      return _gfxQueue;
    }
    QTLAVA_API
    std::shared_ptr<CommandPool> gfxCommandPool( void ) const
    {
      return _cmdPool;
    }
    QTLAVA_API
    vk::Format colorFormat( void ) const
    {
      return _surfaceFormat.format;
    }
    QTLAVA_API
      vk::ColorSpaceKHR colorSpace( void ) const
    {
      return _surfaceFormat.colorSpace;
    }
    QTLAVA_API
    vk::Format depthStencilFormat( void ) const
    {
      return _dsFormat;
    }
    QTLAVA_API
    std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const
    {
      return cmds.at( imageIdx );
    }
    QTLAVA_API
    std::shared_ptr< RenderPass > renderPass( void ) const
    {
      return _dfbFramebuffer->renderPass( );
    }
    QTLAVA_API
    std::shared_ptr< Framebuffer > framebuffer( void ) const
    {
      return _dfbFramebuffer->framebuffer( imageIdx );
    }
  protected:
    QTLAVA_API
    bool event( QEvent* ev ) override;
    QTLAVA_API
    void exposeEvent( QExposeEvent* ev ) override;
    QTLAVA_API
    void resizeEvent( QResizeEvent* ev ) override;

  private:
    bool checkDeviceLost( vk::Result err );
    void addReadback( void );
    void finishBlockingReadback( void );

    void getSurfaceFormats( void );
    void createQueues( void );

    QVulkanInstance inst;
  };
}

#endif /* __QTLAVA_VULKANWINDOW__ */