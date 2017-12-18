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

#include "Geometry.h"
#include "ModelImporter.h"

namespace lava
{
  namespace extras
  {
    Transform::Transform( void )
    {
      SetAsIdentity();
    }

    Transform::Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) :
      position(position),
      rotation(rotation),
      scale(scale)
    {
    }

    Transform::Transform(const glm::vec3& position, const glm::quat& rotation) :
      position(position),
      rotation(rotation),
      scale(glm::vec3(1.0f))
    {
    }

    Transform::Transform(const glm::vec3& position) :
      position(position),
      rotation(glm::quat(glm::vec3(0.0f))),
      scale(glm::vec3(1.0f))
    {
    }

    Transform::~Transform( void )
    {
    }

    void Transform::Translate(glm::vec3 deltaPosition)
    {
      position += deltaPosition;
    }

    void Transform::Rotate(glm::quat deltaRotation)
    {
      rotation *= deltaRotation;
    }

    void Transform::Rotate(glm::vec3 deltaEulerRotationRad)
    {
      glm::quat rotationQuat(deltaEulerRotationRad);
      rotation *= rotationQuat;
    }

    void Transform::Scale(glm::vec3 deltaScale)
    {
      scale *= deltaScale;
    }

    void Transform::SetAsIdentity( void )
    {
      position = glm::vec3(0.0f);
      rotation = glm::quat(glm::vec3(0.0f));
      scale = glm::vec3(1.0f);
    }

    glm::mat4 Transform::GetModelMatrix( void )
    {
      glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), position);
      glm::mat4 matRot = glm::mat4(rotation);
      glm::mat4 matScale = glm::scale(glm::mat4(1.0f), scale);
      glm::mat4 matModel = matTrans * matRot * matScale;

      return matModel;
    }

    Transform Transform::Identity( void )
    {
      Transform result;

      result.SetAsIdentity();

      return result;
    }

    Geometry::Geometry( const DeviceRef& device, const std::string& path )
      : VulkanResource( device )
    {

      lava::extras::ModelImporter mi( path );
      lava::extras::Mesh mesh = mi._meshes[ 0 ];

      _numIndices = mesh.numIndices;

      /*for ( const auto& v: mesh.vertices )
      {
        std::cout << "POSITION: " << v.position.x << ", " << v.position.y << "," << v.position.z << std::endl;
        std::cout << "NORMAL: " << v.normal.x << ", " << v.normal.y << "," << v.normal.z << std::endl;
        std::cout << "TEXCOORD: " << v.texCoord.x << ", " << v.texCoord.y << std::endl;
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~ " << std::endl;
      }*/

      // Vertex buffer
      {
        uint32_t vertexBufferSize = mesh.numVertices * 
          sizeof( lava::extras::Vertex );
        _vbo = std::make_shared<VertexBuffer>( 
          _device, vertexBufferSize );
        _vbo->writeData( 0, vertexBufferSize, 
          mesh.vertices.data( ) );
      }

      // Index buffer
      {
        uint32_t indexBufferSize = _numIndices * sizeof( uint32_t );
        _ibo = std::make_shared<IndexBuffer>( _device, 
          vk::IndexType::eUint32, _numIndices );
        _ibo->writeData( 0, indexBufferSize, mesh.indices.data( ) );
      }
    }
    void Geometry::render( std::shared_ptr<CommandBuffer> cmd, 
      uint32_t numInstance )
    {
      _vbo->bind( cmd );
      _ibo->bind( cmd );
      cmd->drawIndexed( _numIndices, numInstance, 0, 0, 0 );
    }
  }
}