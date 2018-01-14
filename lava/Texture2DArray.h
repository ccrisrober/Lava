#ifndef __LAVA_TEXTURE2DARRAY__
#define __LAVA_TEXTURE2DARRAY__

#include "includes.hpp"

#include "Texture.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class Texture2DArray : public Texture
  {
  public:
    LAVA_API
    // All images on filePaths as same dimensions
    Texture2DArray( const DeviceRef& device, 
      std::vector< std::string >& filePaths,
      const std::shared_ptr<CommandPool>& cmdPool, 
      const std::shared_ptr<Queue>& queue,
      vk::Format format,
      vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
      vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      bool forceLinear = false );
  };
}

#endif /* __LAVA_TEXTURE2DARRAY__ */