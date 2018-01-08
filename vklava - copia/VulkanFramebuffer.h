#ifndef __VKLAVA_VULKANFRAMEBUFFER__
#define __VKLAVA_VULKANFRAMEBUFFER__

#include "includes.hpp"

#include "VulkanResource.h"
//#include "VulkanTexture.h"

namespace lava
{
  struct VULKAN_FRAMEBUFFER_DESC
  {
    uint32_t numColorAttachments;
    std::vector<vk::Format> colorFormats;

    bool hasDepth;
    vk::Format depthFormat;

    uint32_t width;
    uint32_t height;
  };
  class VulkanFramebuffer: public VulkanResource
  {
  public:
    VulkanFramebuffer( VulkanDevicePtr device, const VULKAN_FRAMEBUFFER_DESC& desc );
    ~VulkanFramebuffer( void );
  //protected:
    std::vector< vk::AttachmentDescription > _attachments;
    std::vector< vk::AttachmentReference > _colorReferences;
	vk::AttachmentReference _depthReferences;

	vk::SubpassDescription _subpassDesc;

	vk::SubpassDependency _dependencies[ 2 ];
	vk::RenderPassCreateInfo _renderPassCI;
	vk::FramebufferCreateInfo _framebufferCI;


    std::vector< vk::ImageView > _attachmentViews;




	vk::RenderPass renderPass;
	vk::Framebuffer framebuffer;
  };
}

#endif /* __VKLAVA_VULKANFRAMEBUFFER__ */