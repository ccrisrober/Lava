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

#include "../Device.h"

namespace lava
{
  namespace extras
  {
    Transform::Transform( void )
    {
      SetAsIdentity();
    }

    Transform::Transform( const glm::vec3& position_, const glm::quat& rotation_, 
      const glm::vec3& scale_ ) 
    : position(position_)
      , rotation(rotation_)
      , scale(scale_)
    {
    }

    Transform::Transform( const glm::vec3& position_, 
      const glm::quat& rotation_) 
    : position(position_)
      , rotation(rotation_)
      , scale(glm::vec3(1.0f))
    {
    }

    Transform::Transform(const glm::vec3& position_) 
    : position(position_)
      , rotation(glm::quat(glm::vec3(0.0f)))
      , scale(glm::vec3(1.0f))
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

    Geometry::Geometry( const std::shared_ptr<Device>& device, 
      const std::string& path )
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
        uint32_t vertexBufferSize = mesh.numVertices * sizeof( Vertex );
        _vbo = _device->createVertexBuffer( vertexBufferSize );
        _vbo->writeData( 0, vertexBufferSize, mesh.vertices.data( ) );
      }

      // Index buffer
      {
        uint32_t indexBufferSize = _numIndices * sizeof( uint32_t );
        _ibo = device->createIndexBuffer( vk::IndexType::eUint32, _numIndices );
        _ibo->writeData( 0, indexBufferSize, mesh.indices.data( ) );
      }
    }
    Geometry::Geometry( const std::shared_ptr<Device>& device, 
      const std::shared_ptr<CommandPool> cmdPool, 
      const std::shared_ptr<Queue> queue, const std::string & path )
      : VulkanResource( device )
    {
      lava::extras::ModelImporter mi( path );
      lava::extras::Mesh mesh = mi._meshes[ 0 ];

      _numIndices = mesh.numIndices;

      // Vertex buffer
      {
        uint32_t vertexBufferSize = mesh.numVertices * sizeof( Vertex );
        auto cmd = cmdPool->allocateCommandBuffer( );
        cmd->begin( );

        _vbo = device->createBuffer( vertexBufferSize,
          vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal );
        _vbo->update<Vertex>( cmd, 0, { uint32_t( mesh.vertices.size( ) ),
          mesh.vertices.data( ) } );
        cmd->end( );
        queue->submitAndWait( cmd );
      }
      // Index buffer
      {
        uint32_t indexBufferSize = _numIndices * sizeof( uint32_t );
        auto cmd = cmdPool->allocateCommandBuffer( );
        cmd->begin( );

        _ibo = device->createBuffer( indexBufferSize,
          vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal );
        _ibo->update<uint32_t>( cmd, 0, { uint32_t( mesh.indices.size( ) ),
          mesh.indices.data( ) } );
        cmd->end( );
        queue->submitAndWait( cmd );
      }
    }
    void Geometry::render( std::shared_ptr<CommandBuffer> cmd, 
      uint32_t numInstance )
    {
      cmd->bindVertexBuffer( 0, _vbo, 0 );
      cmd->bindIndexBuffer( _ibo, 0, vk::IndexType::eUint32 );
      cmd->drawIndexed( _numIndices, numInstance, 0, 0, 0 );
    }
  }
}