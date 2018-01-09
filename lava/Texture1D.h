#ifndef __LAVA_TEXTURE1D__
#define __LAVA_TEXTURE1D__

#include "includes.hpp"

#include "Texture.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class Texture1D: public Texture
  {
  public:
    LAVA_API
    Texture1D( const DeviceRef& device, const std::string& filename,
      const std::shared_ptr<CommandPool>& cmdPool, 
      const std::shared_ptr<Queue>& queue,
      vk::Format format,
      vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
      vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      bool forceLinear = false );
  };
}

#endif /* __LAVA_TEXTURE1D__ */