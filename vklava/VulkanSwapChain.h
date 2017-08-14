#pragma once

#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include <assert.h>

#include <algorithm> 
using namespace std;

class VulkanSemaphore : public VulkanResource
{
public:
  VulkanSemaphore( VulkanDevicePtr device )
    : VulkanResource( device )
  {
    VkSemaphoreCreateInfo semaphoreCI;
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCI.pNext = nullptr;
    semaphoreCI.flags = 0;

    VkResult result = vkCreateSemaphore( _device->getLogical( ), 
      &semaphoreCI, nullptr, &_semaphore );
    assert( result == VK_SUCCESS );
  }
  ~VulkanSemaphore( )
  {
    vkDestroySemaphore( _device->getLogical( ), _semaphore, nullptr );
  }

  // Returns the internal handle to the Vulkan object
  VkSemaphore getHandle( ) const
  {
    return _semaphore;
  }

private:
  VkSemaphore _semaphore;
};

class VulkanSwapChain
{
public:
  VulkanSwapChain( );
  ~VulkanSwapChain( );
  void rebuild( VulkanDevicePtr device, VkSurfaceKHR& surface, uint32_t w, 
    uint32_t h, bool vsync, VkFormat colorFormat, VkColorSpaceKHR colorSpace );
  /**
  * Returns the actual width of the swap chain, in pixels. This might differ from the requested size in case it
  * wasn't supported.
  */
  uint32_t getWidth( void ) const
  {
    return _width;
  }

  /**
  * Returns the actual height of the swap chain, in pixels. This might differ from the requested size in case it
  * wasn't supported.
  */
  uint32_t getHeight( void ) const
  {
    return _height;
  }

  operator VkSwapchainKHR( )
  {
    return _swapChain;
  }

  VkSwapchainKHR getSwapChain( void ) const
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


protected:
  VulkanDevicePtr _device;
  uint32_t _width = 0;
  uint32_t _height = 0;
  VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
public:
  VkExtent2D swapchainExtent;
  std::vector<VkImageView> swapChainImageViews;
};

