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
    RenderPass( const DeviceRef& device,
      const std::vector<vk::AttachmentDescription>& attachments,
      const std::vector<vk::SubpassDescription>& subpasses,
      const std::vector<vk::SubpassDependency>& dependencies );
    ~RenderPass( );

    inline operator vk::RenderPass( ) const
    {
      return _renderPass;
    }

  protected:
    vk::RenderPass _renderPass;
  };
}

#endif /* __LAVA_RENDERPASS__ */