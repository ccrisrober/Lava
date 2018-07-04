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

#include "Geometry.h"
#include "ModelImporter.h"
#ifdef LAVA_USE_ASSIMP

namespace lava
{
  namespace utility
  {
    Geometry::Geometry( const std::shared_ptr<Device>& device, 
      const std::string& path, bool adjancency )
      : VulkanResource( device )
    {
      lava::utility::ModelImporter mi( path );
      lava::utility::Mesh mesh = mi._meshes[ 0 ];

      if ( adjancency )
      {
        mesh.convertFacesToAdjancencyFormat( );
      }

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
      const std::shared_ptr<CommandPool>& cmdPool, 
      const std::shared_ptr<Queue>& queue, const std::string& path )
      : VulkanResource( device )
    {
      lava::utility::ModelImporter mi( path );
      lava::utility::Mesh mesh = mi._meshes[ 0 ];

      _numIndices = mesh.numIndices;
      
      auto cmd = cmdPool->allocateCommandBuffer( );
      cmd->begin( );
      // Vertex buffer
      {
        uint32_t vertexBufferSize = mesh.numVertices * sizeof( Vertex );

        _vbo = device->createBuffer( vertexBufferSize,
          vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal );
        _vbo->update<Vertex>( cmd, 0, { uint32_t( mesh.vertices.size( ) ),
          mesh.vertices.data( ) } );
      }
      // Index buffer
      {
        uint32_t indexBufferSize = _numIndices * sizeof( uint32_t );

        _ibo = device->createBuffer( indexBufferSize,
          vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal );
        _ibo->update<uint32_t>( cmd, 0, { uint32_t( mesh.indices.size( ) ),
          mesh.indices.data( ) } );
      }
      cmd->end( );
      queue->submitAndWait( cmd );
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
#endif
