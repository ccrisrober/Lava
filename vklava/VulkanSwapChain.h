#ifndef __VKLAVA_VULKANSWAPCHAIN__
#define __VKLAVA_VULKANSWAPCHAIN__

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanTexture.h"

namespace lava
{
  struct SwapChainSurface
  {
    VulkanImage* image;
    VulkanSemaphore* sync;
    bool acquired;
    bool needsWait;

    //VulkanFramebuffer* framebuffer;
   // VULKAN_FRAMEBUFFER_DESC framebufferDesc;
  };

  class VulkanSwapChain
  {
  public:
    VulkanSwapChain( void );
    ~VulkanSwapChain( void );
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

    /*uint32_t _currentSemaphoreIdx = 0;
    uint32_t _currentBackBufferIdx = 0;
    void acquireBackBuffer( )
    {
    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR( _device->getLogical( ), _swapChain, UINT64_MAX,
    _surfaces[ _currentSemaphoreIdx ].sync->getHandle( ), VK_NULL_HANDLE, &imageIndex );
    assert( result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR );

    // In case surfaces aren't being distributed in round-robin fashion the image and semaphore indices might not match,
    // in which case just move the semaphores
    if ( imageIndex != _currentSemaphoreIdx )
    std::swap( mSurfaces[ _currentSemaphoreIdx ].sync, mSurfaces[ imageIndex ].sync );

    _currentSemaphoreIdx = ( _currentSemaphoreIdx + 1 ) % mSurfaces.size( );

    assert( !mSurfaces[ imageIndex ].acquired && "Same swap chain surface being acquired twice in a row without present()." );
    mSurfaces[ imageIndex ].acquired = true;
    mSurfaces[ imageIndex ].needsWait = true;

    _currentBackBufferIdx = imageIndex;
    }*/

    /** Returns the number of available color surfaces. */


    /*uint32_t getNumColorSurfaces( ) const
    {
      return ( uint32_t ) _surfaces.size( );
    }*/

    /*const SwapChainSurface& getBackBuffer( void )
    {
      return _surfaces[ _currentBackBufferIdx ];
    }*/

    VkSwapchainKHR getHandle( void ) const
    {
      return _swapChain;
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

    //std::vector<SwapChainSurface> _surfaces;
    uint32_t _currentSemaphoreIdx = 0;
    uint32_t _currentBackBufferIdx = 0;
  };
}

#endif /* __VKLAVA_VULKANSWAPCHAIN__ */