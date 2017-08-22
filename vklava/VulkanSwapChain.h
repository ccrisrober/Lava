#ifndef __VKLAVA_VULKANSWAPCHAIN__
#define __VKLAVA_VULKANSWAPCHAIN__

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanTexture.h"

namespace lava
{
  struct SwapChainSurface
  {
    VulkanFramebuffer* framebuffer;
  };

  class VulkanSwapChain
  {
  public:
    VulkanSwapChain( void );
    ~VulkanSwapChain( void );
    void clear( VkSwapchainKHR swapChain );
    void rebuild( VulkanDevicePtr device, VkSurfaceKHR& surface, uint32_t w,
      uint32_t h, bool vsync, VkFormat colorFormat, VkColorSpaceKHR colorSpace,
      bool createDepth, VkFormat depthFormat );
    /**
    * Returns the actual width of the swap chain, in pixels. 
    * This might differ from the requested size in case it wasn't supported.
    */
    uint32_t getWidth( void ) const
    {
      return _width;
    }

    /**
    * Returns the actual height of the swap chain, in pixels. 
    * This might differ from the requested size in case it wasn't supported.
    */
    uint32_t getHeight( void ) const
    {
      return _height;
    }

    operator VkSwapchainKHR( )
    {
      return _swapChain;
    }

    VkSwapchainKHR getHandle( void ) const
    {
      return _swapChain;
    }

    uint32_t _currentSwapImage;
    void begin( )
    {
      // No checking because could be in lost state if change res
    vkAcquireNextImageKHR( _device->getLogical( ), _swapChain, UINT64_MAX, 
      VK_NULL_HANDLE, VK_NULL_HANDLE, &_currentSwapImage );
    }
    void end( VulkanQueue* queue )
    {
      VkPresentInfoKHR info;

      info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      info.pNext = nullptr;
      info.waitSemaphoreCount = 0;
      info.pWaitSemaphores = nullptr;
      info.swapchainCount = 1;
      info.pSwapchains = &_swapChain;
      info.pImageIndices = &_currentSwapImage;
      info.pResults = nullptr;

      vkQueuePresentKHR( queue->getQueue( ), &info );
    }

  protected:
    VulkanDevicePtr _device;
    uint32_t _width = 0;
    uint32_t _height = 0;
    VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
  public:
    std::vector<VkImageView> swapChainImageViews;

    VkImage depthStencilImage;
    //VulkanImage* _depthStencilImage = nullptr;

    std::vector<SwapChainSurface> _surfaces;
    uint32_t _currentSemaphoreIdx = 0;
    uint32_t _currentBackBufferIdx = 0;
  };
}

#endif /* __VKLAVA_VULKANSWAPCHAIN__ */