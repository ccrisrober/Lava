/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#ifndef __POMPEIIUTILS_GEOMETRY__
#define __POMPEIIUTILS_GEOMETRY__

#ifdef POMPEII_USE_ASSIMP

#include <memory>

#include <pompeii/pompeii.h>
#include <pompeiiUtils/api.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace pompeii
{
  namespace utils
  {
    class Geometry : public pompeii::VulkanResource
    {
    public:
      POMPEIIUTILS_API
      Geometry( const std::shared_ptr<Device>& device, const std::string& path, 
        bool adjancency = false );
      POMPEIIUTILS_API
      Geometry( const std::shared_ptr<Device>& device, 
        const std::shared_ptr<CommandPool>& cmdPool, 
        const std::shared_ptr<Queue>& queue, const std::string& path );
      POMPEIIUTILS_API
      void render( std::shared_ptr<CommandBuffer> cmd, uint32_t numInstances = 1 );
    protected:
      std::shared_ptr<Buffer> _vbo;
      std::shared_ptr<Buffer> _ibo;
      uint32_t _numIndices;
    };
  }
}

#endif

#endif /* __POMPEIIUTILS_GEOMETRY__ */
