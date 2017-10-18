#ifndef __LAVA_TEXTURE2D__
#define __LAVA_TEXTURE2D__

#include "includes.hpp"

#include "VulkanResource.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class Texture2D: public VulkanResource
  {
  public:
    LAVA_API
    Texture2D( const DeviceRef& device, const std::string& filename, 
      const std::shared_ptr<CommandPool>& cmdPool, 
      const std::shared_ptr<Queue>& queue,
      vk::Format format = vk::Format::eR8G8B8A8Unorm, 
      bool forceLinear = false );

    LAVA_API
    virtual ~Texture2D( void );

    uint32_t width, height, numChannels;
    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;
    vk::ImageLayout imageLayout;

    vk::ImageView view;
    vk::Sampler sampler;
  };
}

#endif /* __LAVA_TEXTURE2D__ */