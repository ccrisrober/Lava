#ifndef __LAVA_VULKANRENDERER__
#define __LAVA_VULKANRENDERER__

#include <lava/api.h>
#include <lava/CommandBuffer.h>
#include <lava/Framebuffer.h>
#include <lava/PhysicalDevice.h>
#include <lava/Fence.h>
#include <lava/Queue.h>
#include <lava/Swapchain.h>
#include <lava/Window.h>

namespace lava
{
  class VulkanWindowRenderer
  {
  public:
    virtual ~VulkanWindowRenderer( void );

    /**
    * Method called when creating renderer's resources 
    */
    virtual void initResources( void );
    virtual void initSwapChainResources( void );
    virtual void releaseSwapChainResources( void );
    /**
    * Method called when renderer's resources must be released
    */
    virtual void releaseResources( void );
    /**
    * Method called when the draw calls for the next frame are to be added 
    *   to the command buffer
    */
    virtual void nextFrame( void ) = 0;

    //virtual void physicalDeviceLost( void );
    //virtual void logicalDeviceLost( void );
  };

  class VulkanWindow
  {
  private:
    void beginFrame( void );
    void endFrame( void );

    bool createDefaultRenderPass( void );
    void recreateSwapChain( void );
    void releaseSwapChain( void );
    void init( void );
    void reset( void );
  public:
    LAVA_API
    VulkanWindow( void );
    LAVA_API
    ~VulkanWindow( void );

    LAVA_API
    VulkanWindow( const VulkanWindow& ) = delete;
    LAVA_API
    VulkanWindow& operator=( const VulkanWindow& ) = delete;

    LAVA_API
    void setVulkanInstance( const std::shared_ptr< Instance > instance );

    LAVA_API
    void show( void );

    LAVA_API
    inline std::shared_ptr<Window> window( void ) const
    {
      return _window;
    }

    virtual VulkanWindowRenderer* createRenderer( void );
    void frameReady( void );
    std::shared_ptr<PhysicalDevice> physicalDevice( void );
    const vk::PhysicalDeviceProperties* physicalDeviceProperties( void ) const;
    std::shared_ptr<Device> device( void );
    std::shared_ptr<Queue> gfxQueue( void );
    std::shared_ptr<CommandPool> gfxCommandPool( void );
    std::shared_ptr<RenderPass> defaultRenderPass( void );

    vk::Format colorFormat( void ) const;
    vk::Format depthStencilFormat( void ) const;
    glm::ivec2 swapChainImageSize( void ) const;

    std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const;
    std::shared_ptr<Framebuffer> currentFramebuffer( void ) const;
    int currentFrame( void ) const;

    std::shared_ptr<Image> swapChainImage( uint16_t idx ) const;
    std::shared_ptr<ImageView> swapChainImageView( uint16_t idx ) const;

    std::shared_ptr<Image> depthStencilImage( void ) const;
    std::shared_ptr<ImageView> depthStencilImageView( void ) const;

    LAVA_API
    vk::SampleCountFlagBits sampleCountFlagBits( void ) const;

    LAVA_API
    void setSampleCountFlagBits( int sampleCount );

    LAVA_API
    std::vector< int > supportedSampleCounts( void );

    LAVA_API
    glm::mat4 clipCorrectionMatrix( void );

    LAVA_API
    void setWindowTitle( const char*& name );

    LAVA_API
    void setWindowTitle( const std::string& name );

    LAVA_API
    void setDeviceExtensions( const std::vector< std::string >& exts );

  protected:
    std::shared_ptr<Instance> _instance;
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<Device> _device;
    std::shared_ptr<Queue> _gfxQueue;
    std::shared_ptr<Queue> _presQueue;
    std::shared_ptr<CommandPool> _commandPool;
    std::shared_ptr<RenderPass> _defaultRenderPass;
    std::shared_ptr<Swapchain> _swapChain;

    glm::mat4 _clipCorrect;

    vk::Format _colorFormat;
    vk::ColorSpaceKHR _colorSpace;
    vk::Format _dsFormat = vk::Format::eD24UnormS8Uint;

    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

    struct RenderPassResources
    {
      std::shared_ptr<Image> image;
      std::shared_ptr<ImageView> imageView;

      std::shared_ptr<CommandBuffer> cmdBuffer;
      std::shared_ptr<Fence> cmdFence;

      std::shared_ptr<Framebuffer> framebuffer;
    } renderPassResources[ 2 ];

    std::shared_ptr<Image> _depthImage;
    std::shared_ptr<ImageView> _depthView;

    uint16_t _currentFrame = 0;
    glm::ivec2 _swapChainImageSize;

    std::shared_ptr< PipelineCache > pipelineCache;

    std::shared_ptr<Surface> _surface;
    std::shared_ptr<RenderPass> _renderPass;

    bool _framePending = false;

    VulkanWindowRenderer* _renderer = nullptr;

    std::vector<std::string > _requestedDeviceExts;
    std::shared_ptr< Window > _window;

    uint32_t _gfxQueueFamilyIdx;
    uint32_t _presQueueFamilyIdx;

    bool _initialized = false;


    bool _swapChainSupportsReadBack;
  };
}

#endif /* __LAVA_VULKANRENDERER__ */