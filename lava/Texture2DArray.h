#ifndef __LAVA_TEXTURE2DARRAY__
#define __LAVA_TEXTURE2DARRAY__

#include "includes.hpp"

#include "VulkanResource.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class Texture2DArray : public VulkanResource
  {
  public:
    LAVA_API
    // All images on filePaths as same dimensions
    Texture2DArray( const DeviceRef& device, std::vector< std::string >& filePaths,
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue,
      vk::Format format = vk::Format::eR8G8B8A8Unorm,
      bool forceLinear = false );

    LAVA_API
    virtual ~Texture2DArray( void );

    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;
    vk::ImageLayout imageLayout;

    vk::ImageView view;
    vk::Sampler sampler;
  };
}

#endif /* __LAVA_TEXTURE2DARRAY__ */