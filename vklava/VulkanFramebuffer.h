#ifndef __VKLAVA_VULKANFRAMEBUFFER__
#define __VKLAVA_VULKANFRAMEBUFFER__

#include "VulkanResource.h"
#include "VulkanTexture.h"

namespace lava
{
  struct VULKAN_FRAMEBUFFER_DESC
  {
    uint32_t numColorAttachments;
    std::vector<VkFormat> colorFormats;

    bool hasDepth;
    VkFormat depthFormat;

    uint32_t width;
    uint32_t height;
  };
  class VulkanFramebuffer: public VulkanResource
  {
  public:
    VulkanFramebuffer( VulkanDevicePtr device, const VULKAN_FRAMEBUFFER_DESC& desc );
    ~VulkanFramebuffer( void );
  //protected:
    std::vector< VkAttachmentDescription > _attachments;
    std::vector< VkAttachmentReference > _colorReferences;
    VkAttachmentReference _depthReferences;

    VkSubpassDescription _subpassDesc;

    VkSubpassDependency _dependencies[ 2 ];
    VkRenderPassCreateInfo _renderPassCI;
    VkFramebufferCreateInfo _framebufferCI;


    std::vector< VkImageView > _attachmentViews;




    VkRenderPass renderPass;
    VkFramebuffer framebuffer;
  };
}

#endif /* __VKLAVA_VULKANFRAMEBUFFER__ */