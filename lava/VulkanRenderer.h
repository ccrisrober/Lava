#ifndef __LAVA_VULKAN_RENDERER__
#define __LAVA_VULKAN_RENDERER__

#include <lava/api.h>

#include "includes.hpp"
#include "Window.h"
#include "Instance.h"
#include "RenderAPICapabilites.h"
#include "DefaultFramebuffer.h"

namespace lava
{
  class VulkanWindowRenderer
  {
  public:
    LAVA_API
    virtual ~VulkanWindowRenderer( void ) { }

    LAVA_API
    virtual void preInitResources( void ) { }
    LAVA_API
    virtual void initResources( void ) { }
    LAVA_API
    virtual void initSwapChainResources( void ) { }
    LAVA_API
    virtual void releaseSwapChainResources( void ) { }
    LAVA_API
    virtual void releaseResources( void ) { }

    virtual void nextFrame( void ) = 0;

    LAVA_API
    virtual void physicalDeviceLost( void ) { }
    LAVA_API
    virtual void logicalDeviceLost( void ) { }
  };

  class VulkanWindow
  {
  public:
    LAVA_API
    VulkanWindow( void)
    {
      _initialized = false;
      _swapChainImageSize = glm::ivec2( 500, 500 );
    }
    LAVA_API
    virtual ~VulkanWindow( void );

    LAVA_API
    VulkanWindow( const VulkanWindow& ) = delete;
    LAVA_API
    VulkanWindow& operator=( const VulkanWindow& ) = delete;

    LAVA_API
    void setVulkanInstance( const std::shared_ptr< Instance > instance );

    LAVA_API
    bool supportsGrab( void ) const;

    LAVA_API
    void init( void );
    LAVA_API
    void reset( void );

    LAVA_API
    void beginFrame( void );
    LAVA_API
    void endFrame( void );
    LAVA_API
    bool checkDeviceLost( vk::Result res );

    /*LAVA_API
    virtual void createPipelineCache( void ) = 0;
    LAVA_API
    virtual void savePipelineCache( void ) = 0;*/

    LAVA_API
    CommandBufferPtr currentCommandBuffer( void ) const;

    LAVA_API
    RenderAPICapabilities caps( void ) const;

    LAVA_API
    void setWindowTitle( const char*& name );
    LAVA_API
    void setWindowTitle( const std::string& name );

    LAVA_API
    vk::Extent2D getExtent( void ) const
    {
      return _defaultFramebuffer->getExtent( );
    }

    LAVA_API
    std::shared_ptr< PhysicalDevice > physicalDevice( void ) const;
    LAVA_API
    const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const;
    LAVA_API
    std::shared_ptr< Device > device( void ) const;
    LAVA_API
    std::shared_ptr< Queue > graphicQueue( void ) const;
    LAVA_API
    std::shared_ptr< CommandPool > graphicsCommandPool( void ) const;
    LAVA_API
    std::shared_ptr< RenderPass > defaultRenderPass( void ) const;

    LAVA_API
    vk::Format colorFormat( void ) const;
    LAVA_API
    vk::Format depthStencilFormat( void ) const;
    LAVA_API
    glm::ivec2 swapChainImageSize( void ) const;

    int currentFrame( void ) const;

    LAVA_API
    int swapChainImageCount( void ) const;
    LAVA_API
    int currentSwapChainImageIndex( void ) const;
    LAVA_API
    std::shared_ptr< Image > swapChainImage( int idx ) const;
    LAVA_API
    std::shared_ptr< ImageView > swapChainImageView( int idx ) const;
    LAVA_API
    std::shared_ptr< Image > depthStencilImage( void ) const;
    LAVA_API
    std::shared_ptr< ImageView > depthStencilImageView( void ) const;

    LAVA_API
    glm::mat4 clipCorrectionMatrix( void );

    LAVA_API
    virtual void resize( uint32_t width, uint32_t height ) final
    {
      //assert( ( 0 <= width ) && ( 0 <= height ) );

      _swapChainImageSize.x = width;
      _swapChainImageSize.y = height;

      resizeEvent( width, height );
    }

    LAVA_API
    virtual void resizeEvent( uint32_t w, uint32_t h );

    LAVA_API
    void show( void );

    std::shared_ptr< PipelineCache > pipelineCache;

    LAVA_API
    /*std::unique_ptr< */DefaultFramebuffer* /*>*/ defaultFramebuffer( void ) const // TODO: SO UGLY ...
    {
      return _defaultFramebuffer.get( );
    }

    LAVA_API
    std::shared_ptr< Framebuffer > currentFramebuffer( void ) const
    {
      return _defaultFramebuffer->getFramebuffer( );
    }
    LAVA_API
    virtual VulkanWindowRenderer* createRenderer( void );

    LAVA_API
    void frameReady( void );

    LAVA_API
    vk::SampleCountFlagBits sampleCountFlagBits( void ) const;

    LAVA_API
    void setSampleCountFlagBits( int sampleCount );

    LAVA_API
    std::vector< int > supportedSampleCounts( void );

    LAVA_API
    int imagesCount( void ) const;

    LAVA_API
    void setDeviceExtensions( const std::vector< std::string >& extensions );

    virtual void getEnabledFeatures( void )
    {
    }

  private:
    void initCapabilites( void );
    bool createDefaultRenderPass( void );
    void recreateSwapChain( void );

    bool _initialized;

    VulkanWindowRenderer* renderer = nullptr;
    
    std::vector<std::string > _requestedDeviceExts;

  protected:
    int numero = 0;
    std::shared_ptr< Instance > _instance;
    std::shared_ptr< PhysicalDevice > _physicalDevice;
    std::shared_ptr< Device > _device;
    std::shared_ptr< RenderPass > _renderPass;
    
    std::shared_ptr< Surface > _surface;

    std::unique_ptr< DefaultFramebuffer > _defaultFramebuffer;

    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
    

    uint32_t _gfxQueueFamilyIdx;
    uint32_t _presQueueFamilyIdx;
    std::shared_ptr< Queue > _gfxQueue;
    std::shared_ptr< Queue > _presQueue;

    std::shared_ptr< Semaphore > _renderComplete;

    struct ImageResources
    {
      CommandBufferPtr commandBuffer;
    } imageRes[ 2 ];
  public:
    std::shared_ptr< CommandPool > _cmdPool;
    std::shared_ptr< Window > _window;
  protected:
    glm::ivec2 _swapChainImageSize;

    vk::Format _colorFormat;
    vk::ColorSpaceKHR _colorSpace;
    vk::Format _dsFormat;

    //vk::PresentModeKHR presentMode = vk::PresentMode::eFifoKHR;

    glm::mat4 _clipCorrect;

    RenderAPICapabilities _caps;
    uint32_t _currentFrame;
  };
}

#endif /* __LAVA_VULKAN_RENDERER__ */