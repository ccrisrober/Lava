#ifndef __VKLAVA_RENDERWINDOW__
#define __VKLAVA_RENDERWINDOW__

#include <vulkan/vulkan.h>
#include <memory>

#include "VulkanSwapChain.h"

namespace lava
{
  class VulkanRenderAPI;

  class RenderWindow
  {
  public:
    RenderWindow( VulkanRenderAPI& renderAPI );
    ~RenderWindow( void );

    void resize( uint32_t width, uint32_t height );

    //protected:
    VkColorSpaceKHR _colorSpace;
    VkFormat _colorFormat;
    VkFormat _depthFormat;
    std::shared_ptr<VulkanSwapChain> _swapChain;
    VulkanRenderAPI& _renderAPI;
    VkSurfaceKHR _surface;
  };
}

#endif /* __VKLAVA_RENDERWINDOW__ */

