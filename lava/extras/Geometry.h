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

#ifndef __LAVA_GEOMETRY__
#define __LAVA_GEOMETRY__

#include <memory>

#include <lava/Buffer.h>
#include <lava/CommandBuffer.h>
#include <lava/Queue.h>
#include <lava/VulkanResource.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace lava
{
  namespace extras
  {
    class Transform
    {
    public:
      LAVA_API
      Transform( void );
      LAVA_API
      Transform( const glm::vec3& position,
        const glm::quat& rotation, const glm::vec3& scale );
      LAVA_API
      Transform( const glm::vec3& position,
        const glm::quat& rotation );
      LAVA_API
      Transform( const glm::vec3& position );
      LAVA_API
      ~Transform( void );

      LAVA_API
      void Translate( glm::vec3 deltaPosition );
      LAVA_API
      void Rotate( glm::quat deltaRotation );
      LAVA_API
      void Rotate( glm::vec3 deltaEulerRotationRad );
      LAVA_API
      void Scale( glm::vec3 deltaScale );
      LAVA_API
      void SetAsIdentity( void );

      LAVA_API
      glm::mat4 GetModelMatrix( void );

      LAVA_API
      static Transform Identity( void );

      glm::vec3 position;
      glm::quat rotation;
      glm::vec3 scale;
    };
    class Geometry : public VulkanResource
    {
    public:
      LAVA_API
      Geometry( const std::shared_ptr<Device>& device, const std::string& path );
      LAVA_API
      Geometry( const std::shared_ptr<Device>& device, 
        const std::shared_ptr<CommandPool> cmdPool, 
        const std::shared_ptr<Queue> queue, const std::string& path );
      LAVA_API
      void render( std::shared_ptr<CommandBuffer> cmd, uint32_t numInstances = 1 );
    protected:
      std::shared_ptr<Buffer> _vbo;
      std::shared_ptr<Buffer> _ibo;
      uint32_t _numIndices;
    };
  }
}

#endif /* __LAVA_GEOMETRY__ */