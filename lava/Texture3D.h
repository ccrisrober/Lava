#ifndef __LAVA_TEXTURE3D__
#define __LAVA_TEXTURE3D__

#include "includes.hpp"

#include "Texture.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class Texture3D : public Texture
  {
  public:
    LAVA_API
    Texture3D( const DeviceRef& device, uint32_t width, uint32_t height, 
      uint32_t depth, const void* src,
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue, vk::Format format );

    LAVA_API
    void updateData( uint32_t width, uint32_t height, uint32_t depth,
      uint32_t numChannels, const void * data, 
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue, vk::Format format );
    
    uint32_t depth;
  };
}

#endif /* __LAVA_TEXTURE3D__ */