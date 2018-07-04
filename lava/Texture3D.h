/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __LAVA_TEXTURE3D__
#define __LAVA_TEXTURE3D__

#include "includes.hpp"

#include "CommandBuffer.h"
#include "Queue.h"
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
    Texture3D( const std::shared_ptr<Device>& device, uint32_t width, uint32_t height, 
      uint32_t depth, const void* src, uint32_t size,
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue, vk::Format format );

    /*LAVA_API
    void updateData( uint32_t width, uint32_t height, uint32_t depth,
      uint32_t numChannels, const void * data, 
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue );*/
    
    uint32_t depth;
  };
}

#endif /* __LAVA_TEXTURE3D__ */