#ifndef __LAVA_RENDERPASS__
#define __LAVA_RENDERPASS__

#include "includes.hpp"
#include "VulkanResource.h"

namespace lava
{
  class Device;
  class RenderPass : public VulkanResource
  {
  public:
    LAVA_API
    RenderPass( const DeviceRef& device,
      vk::ArrayProxy<const vk::AttachmentDescription> attachments, 
      vk::ArrayProxy<const vk::SubpassDescription> subpasses,
      vk::ArrayProxy<const vk::SubpassDependency> dependencies );
    LAVA_API
    ~RenderPass( void );

    LAVA_API
    inline operator vk::RenderPass( void ) const
    {
      return _renderPass;
    }

  protected:
    vk::RenderPass _renderPass;
  };
}

#endif /* __LAVA_RENDERPASS__ */