#ifndef __VKLAVA_VULKANFRAMEBUFFER__
#define __VKLAVA_VULKANFRAMEBUFFER__

#include "VulkanResource.h"
#include "VulkanTexture.h"

namespace vklava
{
  struct VulkanFramebufferAttachment
  {
    VulkanImage* image = nullptr;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t index = 0;
  };

  class VulkanFramebuffer :
    public VulkanResource
  {
  public:
    VulkanFramebuffer( VulkanDevicePtr device );
    ~VulkanFramebuffer( void );

  protected:
    uint32_t _numAttachments;
    uint32_t _numColorAttachments;
    uint32_t _numLayers;
  };
}

#endif /* __VKLAVA_VULKANFRAMEBUFFER__ */