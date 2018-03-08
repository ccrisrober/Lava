/**
 * Copyright (c) 2017, Lava
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

#ifndef __LAVAUTILS_GEOMETRY__
#define __LAVAUTILS_GEOMETRY__

#include <memory>

#include <lava/lava.h>
#include <lavaUtils/api.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace lava
{
  namespace utility
  {
    class Geometry : public lava::VulkanResource
    {
    public:
      LAVAUTILS_API
      Geometry( const std::shared_ptr<Device>& device, const std::string& path );
      LAVAUTILS_API
      Geometry( const std::shared_ptr<Device>& device, 
        const std::shared_ptr<CommandPool> cmdPool, 
        const std::shared_ptr<Queue> queue, const std::string& path );
      LAVAUTILS_API
      void render( std::shared_ptr<CommandBuffer> cmd, uint32_t numInstances = 1 );
    protected:
      std::shared_ptr<Buffer> _vbo;
      std::shared_ptr<Buffer> _ibo;
      uint32_t _numIndices;
    };
  }
}

#endif /* __LAVAUTILS_GEOMETRY__ */
