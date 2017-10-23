#pragma once

#include "../CommandBuffer.h"
#include <memory>

#include "../Buffer.h"
#include "../VulkanResource.h"

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
      Geometry( const DeviceRef& device, const std::string& path );
      LAVA_API
      void render( std::shared_ptr<CommandBuffer> cmd, uint32_t numInstances = 1 );
    protected:
      std::shared_ptr<VertexBuffer> _vbo;
      std::shared_ptr<IndexBuffer> _ibo;
      uint32_t _numIndices;
    };
  }
}