#ifndef __LAVA_DEFAULTFRAMEBUFFER__
#define __LAVA_DEFAULTFRAMEBUFFER__

#include "includes.hpp"

#include "Device.h"
#include "Surface.h"

namespace lava
{
  class DefaultFramebuffer
  {
  public:
    DefaultFramebuffer( const DeviceRef& device,
      const std::shared_ptr<Surface>& surface,
      vk::Format surfaceFormat, vk::Format depthFormat,
      const std::shared_ptr<RenderPass>& renderPass );

    void rebuild( const DeviceRef& device,
      const std::shared_ptr<Surface>& surface,
      vk::Format surfaceFormat, vk::Format depthFormat,
      const std::shared_ptr<RenderPass>& renderPass );

    const vk::Extent2D& getExtent( void ) const { return _extent; }
    const std::shared_ptr<Framebuffer>& getFramebuffer( void ) const
    {
      return _framebuffers[ _swapchainIndex ];
    }
    void acquireNextFrame( uint64_t timeout = UINT64_MAX, const std::shared_ptr<Fence>& fence = {} )
    {
      _swapchainIndex = _swapchain->acquireNextImage( timeout, fence );
    }

    const std::shared_ptr<Semaphore>& getPresentSemaphore( void ) const
    {
      return _swapchain->getPresentCompleteSemaphores( )[ _swapchainIndex ];
    }

    void present( const std::shared_ptr<Queue>& queue,
      vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores = {} )
    {
      auto results = queue->present( waitSemaphores, _swapchain, _swapchainIndex );
    }
  private:
    void clear( vk::SwapchainKHR swapChain );

    vk::Extent2D _extent;
    std::shared_ptr<Swapchain> _swapchain;
    uint32_t _swapchainIndex;
    std::vector<std::shared_ptr<Image>> _colorImages;
    std::vector<std::shared_ptr<ImageView>> _colorViews;
    std::shared_ptr<Image> _depthImage;
    std::shared_ptr<ImageView> _depthView;
    std::vector<std::shared_ptr<Framebuffer>> _framebuffers;
  };
}

#endif /* __LAVA_DEFAULTFRAMEBUFFER__ */