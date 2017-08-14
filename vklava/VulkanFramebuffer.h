#pragma once
#include "VulkanResource.h"
#include "VulkanTexture.h"

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

