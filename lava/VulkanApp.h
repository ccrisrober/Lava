#ifndef __LAVA_VULKANAPP__
#define __LAVA_VULKANAPP__

#include <lava/api.h>

#include "includes.hpp"
#include "Window.h"
#include "Instance.h"
#include "RenderAPICapabilites.h"
#include "DefaultFramebuffer.h"

namespace lava
{
  class VulkanApp
  {
  public:
    LAVA_API
      VulkanApp( const char* title, uint32_t width, uint32_t height );
    LAVA_API
      virtual ~VulkanApp( void );

    LAVA_API
    inline const RenderAPICapabilities& getCapabilities( void ) const
    {
      return caps;
    }

    LAVA_API
    void waitEvents( void )
    {
      glfwWaitEvents( );
    }
    LAVA_API
    bool isRunning( void )
    {
      return getWindow( )->isRunning( );
      //return !glfwWindowShouldClose( getWindow( ) );
    }

    LAVA_API
    void paint( void );
    LAVA_API
    const std::shared_ptr<Window> getWindow( ) const { return _window; }
  protected:
    LAVA_API
    virtual void resize( uint32_t w, uint32_t h );

    LAVA_API
    virtual void doResize( uint32_t width, uint32_t height );
    LAVA_API
    virtual void doPaint( void );

    LAVA_API
    virtual void cursorPosEvent( double xPos, double yPos );
    LAVA_API
    virtual void keyEvent( int key, int scancode, int action, int mods );
    LAVA_API
    virtual void mouseButtonEvent( int button, int action, int mods );

    LAVA_API
    inline const vk::Format getColorFormat( void ) const
    {
      return _colorFormat;
    }

  private:
    static void paintCallback( GLFWwindow *w );
    static void resizeCallback( GLFWwindow *w, int width, int height );
    static void cursorPosCallback( GLFWwindow * w, double xPos, double yPos );
    static void keyCallback( GLFWwindow* w, int key, int code,
      int action, int mods );
    static void mouseButtonCallback( GLFWwindow * w, int button,
      int action, int mods );

  protected:
    std::shared_ptr< Window > _window;
    std::shared_ptr<Instance> instance;

    std::shared_ptr<PhysicalDevice> _physicalDevice;
    DeviceRef _device;
    std::unique_ptr<DefaultFramebuffer> _defaultFramebuffer;
    std::shared_ptr<Queue> _graphicsQueue;
    std::shared_ptr<RenderPass> _renderPass;
    std::shared_ptr<Surface> _surface;

    uint32_t _queueFamilyIndex;
    vk::ColorSpaceKHR _colorSpace;
    vk::Format _colorFormat;
    vk::Format _depthFormat;
    std::shared_ptr<Semaphore> _renderComplete;

    RenderAPICapabilities caps;

    void initCapabilities( void );
    bool checkValidationLayerSupport(
      const std::vector<const char*>& validationLayers );
  };
}

#endif /* __LAVA_VULKANAPP__ */