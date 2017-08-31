#ifndef __LAVA_SWAPCHAIN__
#define __LAVA_SWAPCHAIN__

#include "includes.hpp"
#include "Image.h"
#include "Semaphore.h"
#include "VulkanResource.h"

namespace lava
{
  class Device;
  class Surface;
  class Fence;
  class Swapchain : public VulkanResource
  {
  public:
    Swapchain( const DeviceRef& device, const std::shared_ptr<Surface>& surface,
      uint32_t numImageCount, vk::Format imageFormat, vk::ColorSpaceKHR colorSpace,
      const vk::Extent2D& imageExtent, uint32_t imageArrayLayers,
      vk::ImageUsageFlags imageUsage, vk::SharingMode imageSharingMode,
      const std::vector<uint32_t>& queueFamilyIndices,
      vk::SurfaceTransformFlagBitsKHR preTransform,
      vk::CompositeAlphaFlagBitsKHR compositeAlpha, vk::PresentModeKHR presentMode,
      bool clipped, const std::shared_ptr<Swapchain>& oldSwapchain );
    virtual ~Swapchain( );

    const std::vector<std::shared_ptr<Image>>& getImages( void ) const;
    const std::vector<std::shared_ptr<Semaphore>>& getPresentCompleteSemaphores( ) const
    {
      return _presentCompleteSemaphores;
    }

    inline operator vk::SwapchainKHR( void ) const
    {
      return _swapchain;
    }

    uint32_t acquireNextImage( uint64_t timeout = UINT64_MAX,
      const std::shared_ptr<Fence>& fence = {} );

  protected:
    std::vector<std::shared_ptr<Image>> _images;
    std::vector<std::shared_ptr<Semaphore>> _presentCompleteSemaphores;
    vk::SwapchainKHR _swapchain;
    std::shared_ptr<Semaphore> _freeSemaphore;
  };
}

#endif /* __LAVA_SWAPCHAIN__ */