#ifndef __LAVA_TEXTURE_CUBEMAP__
#define __LAVA_TEXTURE_CUBEMAP__

#include "includes.hpp"

#include "VulkanResource.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class TextureCubemap: public VulkanResource
  {
  public:
    LAVA_API
    TextureCubemap( const DeviceRef& device, 
      const std::array< std::string, 6 >& filePaths, 
      const std::shared_ptr<CommandPool>& cmdPool, 
      const std::shared_ptr<Queue>& queue, 
      vk::Format format = vk::Format::eR8G8B8A8Unorm, 
      bool forceLinear = false );

    LAVA_API
    virtual ~TextureCubemap( void );

    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;
    vk::ImageLayout imageLayout;

    vk::ImageView view;
    vk::Sampler sampler;
  };
}

#endif /* __LAVA_TEXTURE_CUBEMAP__ */