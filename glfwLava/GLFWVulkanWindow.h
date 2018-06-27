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

  class SwapChain
  {
  public:
    SwapChain( const std::shared_ptr< lava::Device >& device,
      const vk::SurfaceKHR& surface, uint32_t presQueueFamilyIdx,
      const vk::Extent2D& desiredExtent )
    {
      this->_device = device;
      this->_surface = surface;
      this->present_queue_family_index = presQueueFamilyIdx;
      this->desired_extent = desiredExtent;

      createSwapChain( );
      createImageViews( );
    }
    ~SwapChain( void )
    {
      cleanup( true );
    }
    inline operator vk::SwapchainKHR( void ) const
    {
      return _swapchain;
    }
    void resize( const vk::Extent2D& extent )
    {
      this->desired_extent = extent;
      recreate( );
    }
    const vk::Extent2D& extent( void ) const
    {
      return desired_extent;
    }
    const vk::Format colorFormat( void ) const
    {
      return _format;
    }
    const std::vector< std::shared_ptr< ImageView> > imageViews( void ) const
    {
      return this->_imageViews;
    }
    void recreate( void )
    {
      _device->waitIdle( );
      cleanup( false );
      createSwapChain( );
      createImageViews( );
    }
  private:
    void createSwapChain( void )
    {
      auto physicalDevice = static_cast<vk::PhysicalDevice>(
        *_device->getPhysicalDevice( ) );
      auto surfaceCaps = physicalDevice.getSurfaceCapabilitiesKHR( _surface );
      auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR( _surface );
      auto surfacePresentModes = physicalDevice.getSurfacePresentModesKHR( _surface );

      vk::Extent2D extent;
      // If width/height is 0xFFFFFFFF, we can manually specify width, height
      if ( surfaceCaps.currentExtent.width != std::numeric_limits<uint32_t>::max( ) )
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

      desired_extent = extent; // TODO ??

      // Find present mode
      auto presentModes = vk::PhysicalDevice(
        *_device->getPhysicalDevice( ) ).getSurfacePresentModesKHR( _surface );
      vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

      auto imageCount = surfaceCaps.minImageCount + 1;
      if ( surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount )
      {
        imageCount = surfaceCaps.maxImageCount;
      }
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

      bool _swapChainSupportsReadBack = !!( surfaceCaps.supportedUsageFlags &
        vk::ImageUsageFlagBits::eTransferSrc );

      if ( _swapChainSupportsReadBack )
      {
        usage |= vk::ImageUsageFlagBits::eTransferSrc;
      }

      vk::SwapchainKHR oldSwapchain = _swapchain;

      vk::SurfaceFormatKHR surfaceFormat = surfaceFormats[ 0 ];

      auto sci = vk::SwapchainCreateInfoKHR( )
        .setSurface( _surface )
        .setMinImageCount( imageCount )
        .setImageFormat( surfaceFormat.format )
        .setImageColorSpace( surfaceFormat.colorSpace )
        .setImageExtent( extent )
        .setImageArrayLayers( 1 )
        .setImageUsage( usage );

      // todo. sharing mode
      sci.setImageSharingMode( vk::SharingMode::eExclusive )
        .setCompositeAlpha( compositeAlpha )
        .setPresentMode( presentMode )
        .setClipped( VK_TRUE )
        .setPreTransform( surfaceTransform )
        .setOldSwapchain( oldSwapchain );

      _swapchain = static_cast< vk::Device >( *_device ).createSwapchainKHR( sci );

      if ( oldSwapchain )
      {
        static_cast< vk::Device >( *_device ).destroySwapchainKHR( oldSwapchain );
      }
      _images.clear( );
      std::vector<vk::Image> images =
        static_cast< vk::Device >( *_device ).getSwapchainImagesKHR( _swapchain );
      _images.reserve( images.size( ) );
      for ( size_t i = 0, l = images.size( ); i < l; ++i )
      {
        _images.push_back( std::make_shared<Image>( _device, images[ i ] ) );
      }
      _format = surfaceFormat.format;
    }
    void createImageViews( void )
    {
      uint32_t l = _images.size( );
      _imageViews.resize( l );
      for ( uint32_t i = 0; i < l; ++i )
      {
        _imageViews[ i ] = _images[ i ]->createImageView(
          vk::ImageViewType::e2D,
          _format,
          {
            vk::ComponentSwizzle::eR,
            vk::ComponentSwizzle::eG,
            vk::ComponentSwizzle::eB,
            vk::ComponentSwizzle::eA
          },
          { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        );
      }
    }
    void cleanup( bool destroySwapchain )
    {
      std::cout << "Destroying image views" << std::endl;
      for ( auto& iv : _imageViews )
      {
        iv.reset( );
      }
      if ( destroySwapchain )
      {
        std::cout << "Destroying swap chain" << std::endl;
        static_cast< vk::Device >( *_device ).destroySwapchainKHR( _swapchain );
        _swapchain = nullptr;
      }
    }
  protected:
    std::shared_ptr< lava::Device > _device;
    vk::SurfaceKHR _surface;
    vk::SwapchainKHR _swapchain;
    uint32_t present_queue_family_index;
    vk::Extent2D desired_extent;
    vk::Format _format;

    std::vector< std::shared_ptr< Image > > _images;
    std::vector< std::shared_ptr< ImageView > > _imageViews;
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
        { _swapchain->imageViews( )[ i ],
          _depthView }, extent, 1 ) );
      }

      std::cout << "Framebuffer Swapchain OK" << std::endl;

    }
    
    void createRenderPasses( void )
    {
      // Creating renderpass
      {
        std::vector< vk::AttachmentDescription > attDesc =
        {
          vk::AttachmentDescription( // attachment 0 (color render target)
            vk::AttachmentDescriptionFlagBits( ), _swapchain->colorFormat( ), 
            vk::SampleCountFlagBits::e1, //sampleCount,
            vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, // color
            vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, // stencil
            vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
          ),
          vk::AttachmentDescription( // attachment 1 (depth render target)
            vk::AttachmentDescriptionFlagBits( ), depthFormat,  
            vk::SampleCountFlagBits::e1, //sampleCount,
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
      vk::SurfaceKHR surface, uint32_t presQueueFamilyIdx, 
      vk::Extent2D extent, vk::Format depthFormat )
      : _device( dev)
      , depthFormat( depthFormat )
    {
      _swapchain = std::make_shared< SwapChain >( _device, surface, presQueueFamilyIdx, extent );
      
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
        vk::SampleCountFlagBits::e1, tiling,
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
    const std::shared_ptr< SwapChain > swapchain( void ) const
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
    GLFWLAVA_API
    const std::shared_ptr< Framebuffer > framebuffer( void ) const
    {
      static short currentIdx = 0;
      auto fbo = _framebuffers.at( currentIdx );
      ++currentIdx;
      currentIdx = currentIdx % _framebuffers.size( );
      return fbo;
    }
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
        vk::SampleCountFlagBits::e1, tiling,
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
    std::shared_ptr< SwapChain > _swapchain;
    std::vector<std::shared_ptr<Framebuffer>> _framebuffers;
    std::shared_ptr< Image > _depthImage;
    std::shared_ptr< ImageView > _depthView;
    std::shared_ptr< RenderPass > _renderPass;
    vk::Format depthFormat;
  };

  class GLFWVulkanWindow
  {
  private:
    GLFWwindow* window;
    static void OnWindowResized( GLFWwindow *window, int width, int height );

    vk::SurfaceKHR _surface;

    std::shared_ptr<lava::Instance> _instance;
    std::shared_ptr<lava::PhysicalDevice> _physicalDevice;
    std::shared_ptr<lava::Device> _device;

    std::shared_ptr<lava::Semaphore> imageAvailableSem;
    std::shared_ptr<lava::Semaphore> renderFinishedSem;

    vk::SurfaceFormatKHR _surfaceFormat;
    vk::Format _dsFormat;

    std::vector<std::string > _requestedDeviceExts;
    uint32_t _gfxQueueFamilyIdx;
    uint32_t _presQueueFamilyIdx;
    std::shared_ptr< lava::Queue > _gfxQueue;
    std::shared_ptr< lava::Queue > _presQueue;


    DefaultFramebuffer* _dfbFramebuffer = nullptr;


    std::shared_ptr< lava::CommandPool > _cmdPool;


    virtual void InitVulkan( bool enableLayers );
    virtual void InitWindow( int width, int height, const std::string& title );
    virtual void OnWindowResized( int width, int height );

    void createSurface( void );
    void recreateSwapchain( void );
    void cleanup( );

    GLFWVulkanWindowRenderer* renderer = nullptr;
  protected:
    virtual GLFWVulkanWindowRenderer* createRenderer( void );
  public:
    GLFWLAVA_API
    GLFWVulkanWindow( int width, int height, const std::string& title, bool enableLayers );

    GLFWLAVA_API
    ~GLFWVulkanWindow( void );

    GLFWLAVA_API
    void beginFrame( void );
    GLFWLAVA_API
    void update( void );
    GLFWLAVA_API
    void endFrame( void );

    GLFWLAVA_API
    void render( void );

    void drawFrame( std::uint32_t image_index,
      std::vector<vk::Semaphore> wait_semaphores,
      std::vector<vk::PipelineStageFlags> wait_stages,
      std::vector<vk::Semaphore> signal_semaphores );

    GLFWLAVA_API
    GLFWwindow* getWindow( void ) const { return window; }

    std::shared_ptr<CommandBuffer> cmd;
  };
}