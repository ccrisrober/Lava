#ifndef __LAVA_TEXTURE__
#define __LAVA_TEXTURE__

#include "includes.hpp"

#include "VulkanResource.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class Texture: public VulkanResource
  {
  public:
    LAVA_API
    Texture( const DeviceRef& device );
    vk::Image image;
    vk::ImageLayout imageLayout;
    vk::DeviceMemory deviceMemory;
    vk::ImageView view;
    vk::Sampler sampler;
    uint32_t width, height;
    uint32_t mipLevels;
    uint32_t layerCount;
    vk::DescriptorImageInfo descriptor;
  };
}

#endif /* __LAVA_TEXTURE__ */