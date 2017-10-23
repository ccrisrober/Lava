#ifndef __LAVA_FRAMEBUFFER__
#define __LAVA_FRAMEBUFFER__

#include <lava/api.h>

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace lava
{
  class Device;
  class RenderPass;
  class ImageView;
  class Framebuffer : public VulkanResource, private NonCopyable<Framebuffer>
  {
  public:
    LAVA_API
    Framebuffer( const DeviceRef& device, const std::shared_ptr<RenderPass>& renderPass,
      const std::vector<std::shared_ptr<ImageView>>& attachments, 
      const vk::Extent2D& extent, uint32_t layers );
    LAVA_API
    virtual ~Framebuffer( void );

    inline operator vk::Framebuffer( void ) const
    {
      return _framebuffer;
    }

  private:
    std::shared_ptr<RenderPass> _renderPass;
    vk::Framebuffer _framebuffer;
    std::vector<std::shared_ptr<ImageView>> _attachments;
  };
}

#endif /* __LAVA_FRAMEBUFFER__ */