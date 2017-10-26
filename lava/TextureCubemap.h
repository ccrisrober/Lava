#ifndef __LAVA_TEXTURE_CUBEMAP__
#define __LAVA_TEXTURE_CUBEMAP__

#include "includes.hpp"

#include "Texture.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class TextureCubemap: public Texture
  {
  public:
    LAVA_API
    TextureCubemap( const DeviceRef& device, 
      const std::array< std::string, 6 >& filePaths,
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue, vk::Format format,
      vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
      vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      bool forceLinear = false );
  };
}

#endif /* __LAVA_TEXTURE_CUBEMAP__ */