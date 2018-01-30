#ifndef __QTLAVA_VULKANWINDOW__
#define __QTLAVA_VULKANWINDOW__

#include <lava/lava.h>
#include <qtLava/api.h>

#include <QWindow>

namespace lava
{
  class QVulkanWindowRenderer
  {
  public:
    QTLAVA_API
    virtual ~QVulkanWindowRenderer( void );

    /**
    * Method called when creating renderer's resources
    */
    QTLAVA_API
    virtual void initResources( void );
    QTLAVA_API
    virtual void initSwapChainResources( void );
    QTLAVA_API
    virtual void releaseSwapChainResources( void );
    /**
    * Method called when renderer's resources must be released
    */
    QTLAVA_API
    virtual void releaseResources( void );
    /**
    * Method called when the draw calls for the next frame are to be added
    *   to the command buffer
    */
    QTLAVA_API
    virtual void nextFrame( void ) = 0;

    //virtual void physicalDeviceLost( void );
    //virtual void logicalDeviceLost( void );
  };

  class QVulkanWindow : public QWindow
  {
  public:
    explicit QVulkanWindow( QWindow* parent = nullptr );
    ~QVulkanWindow( void );

    QTLAVA_API
    void beginFrame( void );
    QTLAVA_API
    void endFrame( void );
    QTLAVA_API
    std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const;
    QTLAVA_API
    std::shared_ptr< PhysicalDevice > physicalDevice( void ) const;
    QTLAVA_API
    const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const;
    QTLAVA_API
    std::shared_ptr< Device > device( void ) const;
    QTLAVA_API
    std::shared_ptr< Queue > gfxQueue( void ) const;
    QTLAVA_API
    std::shared_ptr< CommandPool > gfxCommandPool( void ) const;
    QTLAVA_API
    std::shared_ptr< RenderPass > defaultRenderPass( void ) const;

    QTLAVA_API
    vk::Format colorFormat( void ) const;
    QTLAVA_API
    vk::Format depthStencilFormat( void ) const;

    QTLAVA_API
    virtual QVulkanWindowRenderer* createRenderer( void );

  private:
    QVulkanWindowRenderer* renderer = nullptr;
  protected:
    std::shared_ptr< Instance > _instance;
    std::shared_ptr< PhysicalDevice > _physicalDevice;
    std::shared_ptr< Device > _device;
    std::shared_ptr< RenderPass > _renderPass;

    std::shared_ptr< Surface > _surface;

    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

    uint32_t _gfxQueueFamilyIdx;
    uint32_t _presQueueFamilyIdx;
    std::shared_ptr< Queue > _gfxQueue;
    std::shared_ptr< Queue > _presQueue;

    struct ImageResources
    {
      std::shared_ptr<CommandBuffer> commandBuffer;
    } imageRes[ 2 ];

    std::shared_ptr< CommandPool > _cmdPool;
    vk::Format _colorFormat;
    vk::ColorSpaceKHR _colorSpace;
    vk::Format _dsFormat;

  };
}

#endif /* __QTLAVA_VULKANWINDOW__ */