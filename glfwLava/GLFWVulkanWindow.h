#pragma once

#include <lava/lava.h>
#include <glfwLava/api.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace lava
{
  class GLFWVulkanWindowRenderer
  {
  public:
    GLFWLAVA_API
    virtual ~GLFWVulkanWindowRenderer( void ) { }

    /**
    * Method called when creating renderer's resources
    */
    GLFWLAVA_API
    virtual void initResources( void ) { }
    GLFWLAVA_API
    virtual void initSwapChainResources( void ) { }
    GLFWLAVA_API
    virtual void releaseSwapChainResources( void ) { }
    /**
    * Method called when renderer's resources must be released
    */
    GLFWLAVA_API
    virtual void releaseResources( void ) { }
    /**
    * Method called when the draw calls for the next frame are to be added
    *   to the command buffer
    */
    GLFWLAVA_API
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
    GLFWLAVA_API
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
    
    GLFWLAVA_API
    ~DefaultFramebuffer( void )
    {
      cleanupFramebuffers( );
      cleanupRenderPasses( );
      _depthView.reset( );
      _depthImage.reset( );
      _swapchain.reset( );
    }
    
    GLFWLAVA_API
    const vk::Extent2D& extent( void ) const
    {
      return _swapchain->extent( );
    }
    
    GLFWLAVA_API
    const std::shared_ptr< Swapchain > swapchain( void ) const
    {
      return _swapchain;
    }
    
    GLFWLAVA_API
    const std::shared_ptr< RenderPass > renderPass( void ) const
    {
      return _renderPass;
    }
    
    GLFWLAVA_API
    const std::shared_ptr< Framebuffer > framebuffer( short idx ) const
    {
      return _framebuffers.at( idx );
    }
    
    /*GLFWLAVA_API
    const std::shared_ptr< Framebuffer > framebuffer( void ) const
    {
      static short currentIdx = 0;
      auto fbo = _framebuffers.at( currentIdx );
      ++currentIdx;
      currentIdx = currentIdx % _framebuffers.size( );
      return fbo;
    }*/
    
    GLFWLAVA_API
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

  class GLFWVulkanWindow
  {
  protected:
    GLFWLAVA_API
    virtual bool setupPipelineCache( void );
  private:
    GLFWwindow* window;
    static void OnWindowResized( GLFWwindow *window, int width, int height );

    void initVulkan( void );
    void cleanupVulkan( void );
    void recreateSwapchain( void );
    bool _initialized;

    std::shared_ptr< lava::Instance > _instance = nullptr;
    std::shared_ptr< lava::PhysicalDevice > _physicalDevice = nullptr;
    std::shared_ptr< lava::Device > _device = nullptr;

    std::shared_ptr< lava::Surface > surface = nullptr;


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
    //QImage _frameGrabTargetImage;
    std::shared_ptr< lava::Image > frameGrabImage = nullptr;
  protected:
    std::shared_ptr< PipelineCache > _pipelineCache;
  private:
    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

    std::shared_ptr< lava::Semaphore > _renderComplete = nullptr;
    GLFWVulkanWindowRenderer* renderer = nullptr;

  protected:
    GLFWLAVA_API
    virtual GLFWVulkanWindowRenderer* createRenderer( void );
  public:
    GLFWLAVA_API
    void requestUpdate( void )
    {
      glfwPollEvents( );
    }

    //GLFWLAVA_API
    //virtual void setVkInstance( const vk::Instance& instance );
    GLFWLAVA_API
    explicit GLFWVulkanWindow( int width, int height, 
      const std::string& title, bool enableLayers );
    GLFWLAVA_API
    GLFWVulkanWindow( const GLFWVulkanWindow& ) = delete;
    GLFWLAVA_API
    GLFWVulkanWindow( GLFWVulkanWindow&& ) = delete;
    GLFWLAVA_API
    ~GLFWVulkanWindow( void );

    GLFWLAVA_API
    GLFWVulkanWindow& operator=( const GLFWVulkanWindow& ) = delete;
    GLFWLAVA_API
    GLFWVulkanWindow& operator=( GLFWVulkanWindow&& ) = delete;

    GLFWLAVA_API
    const std::shared_ptr< PipelineCache > pipelineCache( void )
    {
      return _pipelineCache;
    }
  protected:
    //GLFWLAVA_API
    void beginFrame( void );
    //GLFWLAVA_API
    void endFrame( void );
  public:
    GLFWLAVA_API
    void frameReady( void );

   // GLFWLAVA_API
    //QImage grab( void );

    GLFWLAVA_API
    const bool supportGrab( void )
    {
      return //_dfbFramebuffer == nullptr ? false : 
        _dfbFramebuffer->swapchain( )->swapchainSupportsReadBack( );
    }

    GLFWLAVA_API
    GLFWwindow* getWindow( void ) const { return window; }

    GLFWLAVA_API
    vk::Extent2D swapchainImageSize( void ) const
    {
      int w, h;
      glfwGetWindowSize( window, &w, &h );
      return vk::Extent2D( w, h );
    }

    GLFWLAVA_API
    vk::SampleCountFlagBits sampleCountFlagBits( void ) const
    {
      return sampleCount;
    }
    GLFWLAVA_API
    void setSampleCountFlagBits( int sampleCount );
    GLFWLAVA_API
    std::vector< int > supportedSampleCounts( void );

    GLFWLAVA_API
    std::shared_ptr<PhysicalDevice> physicalDevice( void ) const
    {
      return _physicalDevice;
    }
    GLFWLAVA_API
    const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const
    {
      return physicalDevice( )->getDeviceProperties( );
    }
    GLFWLAVA_API
    std::shared_ptr<Device> device( void ) const
    {
      return _device;
    }
    GLFWLAVA_API
    std::shared_ptr<Queue> gfxQueue( void ) const
    {
      return _gfxQueue;
    }
    GLFWLAVA_API
    std::shared_ptr<CommandPool> gfxCommandPool( void ) const
    {
      return _cmdPool;
    }
    GLFWLAVA_API
    vk::Format colorFormat( void ) const
    {
      return _surfaceFormat.format;
    }
    GLFWLAVA_API
      vk::ColorSpaceKHR colorSpace( void ) const
    {
      return _surfaceFormat.colorSpace;
    }
    GLFWLAVA_API
    vk::Format depthStencilFormat( void ) const
    {
      return _dsFormat;
    }
    GLFWLAVA_API
    std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const
    {
      return cmds.at( imageIdx );
    }
    GLFWLAVA_API
    std::shared_ptr< RenderPass > renderPass( void ) const
    {
      return _dfbFramebuffer->renderPass( );
    }
    GLFWLAVA_API
    std::shared_ptr< Framebuffer > framebuffer( void ) const
    {
      return _dfbFramebuffer->framebuffer( imageIdx );
    }

    GLFWLAVA_API
    virtual void keyEvent( int key, int act ) { };
    GLFWLAVA_API
    virtual void mouseEvent( double xPos, double yPos ) { }
    GLFWLAVA_API
    virtual void scrollEvent( double xOff, double yOff ) { }
    //GLFWLAVA_API
    //void mouseEvent( int key, int act ) { };
  private:
    bool checkDeviceLost( vk::Result err );
    void addReadback( void );
    void finishBlockingReadback( void );

    void getSurfaceFormats( void );
    void createQueues( void );

  public:
    GLFWLAVA_API
    void show( void );
  };
}