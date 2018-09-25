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

#ifndef __TEAPOT_QUAD__
#define __TEAPOT_QUAD__

#include <pompeii/pompeii.h>
#include "teapotdata.h"

class TeapotQuad
{
public:
  TeapotQuad( std::shared_ptr< Device > device,
    std::shared_ptr< CommandPool > cmdPool,
    std::shared_ptr< Queue > queue )
  {
    int verts = 32 * 16;
    float * v = new float[ verts * 3 ];

    generatePatches( v );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = verts * sizeof( float );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, v );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = cmdPool->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, vertexBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      queue->submitAndWait( cmd );
    }
    delete[ ] v;
  }
  void render( std::shared_ptr< CommandBuffer > cmd )
  {
    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->draw( 512, 1, 0, 0 );
  }
protected:
  void generatePatches(float * v)
  {
    int idx = 0;

    // Build each patch
    // The rim
    buildPatchReflect( 0, v, idx, true, true );
    // The body
    buildPatchReflect( 1, v, idx, true, true );
    buildPatchReflect( 2, v, idx, true, true );
    // The lid
    buildPatchReflect( 3, v, idx, true, true );
    buildPatchReflect( 4, v, idx, true, true );
    // The bottom
    buildPatchReflect( 5, v, idx, true, true );
    // The handle
    buildPatchReflect( 6, v, idx, false, true );
    buildPatchReflect( 7, v, idx, false, true );
    // The spout
    buildPatchReflect( 8, v, idx, false, true );
    buildPatchReflect( 9, v, idx, false, true );
  }
  void buildPatchReflect( int patchNum,
    float *v, int &index,
    bool reflectX, bool reflectY )
  {
    glm::vec3 patch[ 4 ][ 4 ];
    glm::vec3 patchRevV[ 4 ][ 4 ];
    getPatch( patchNum, patch, false );
    getPatch( patchNum, patchRevV, true );

    // Patch without modification
    buildPatch( patchRevV, v, index, glm::mat3( 1.0f ) );

    // Patch reflected in x
    if ( reflectX )
    {
      buildPatch( patch, v,
        index, glm::mat3( glm::vec3( -1.0f, 0.0f, 0.0f ),
          glm::vec3( 0.0f, 1.0f, 0.0f ),
          glm::vec3( 0.0f, 0.0f, 1.0f ) ) );
    }

    // Patch reflected in y
    if ( reflectY )
    {
      buildPatch( patch, v,
        index, glm::mat3( glm::vec3( 1.0f, 0.0f, 0.0f ),
          glm::vec3( 0.0f, -1.0f, 0.0f ),
          glm::vec3( 0.0f, 0.0f, 1.0f ) ) );
    }

    // Patch reflected in x and y
    if ( reflectX && reflectY )
    {
      buildPatch( patchRevV, v,
        index, glm::mat3( glm::vec3( -1.0f, 0.0f, 0.0f ),
          glm::vec3( 0.0f, -1.0f, 0.0f ),
          glm::vec3( 0.0f, 0.0f, 1.0f ) ) );
    }
  }
  void buildPatch(glm::vec3 patch[][4],
          float *v, int &index, glm::mat3 reflect)
  {
    for( int i = 0; i < 4; ++i )
    {
      for( int j = 0 ; j < 4; ++j )
      {
        glm::vec3 pt = reflect * patch[i][j];

        v[index] = pt.x;
        v[index + 1] = pt.y;
        v[index + 2] = pt.z;

        index += 3;
      }
    }
  }
  void getPatch( int patchNum, glm::vec3 patch[][4], bool reverseV )
  {
    for( int u = 0; u < 4; ++u)
    {          // Loop in u direction
      for( int v = 0; v < 4; ++v )
      {     // Loop in v direction
        if( reverseV )
        {
          patch[u][v] = glm::vec3(
            Teapot::cpdata[Teapot::patchdata[patchNum]
              [u * 4 + (3 - v)]][ 0 ],
            Teapot::cpdata[Teapot::patchdata[patchNum]
              [u * 4 + (3 - v)]][ 1 ],
            Teapot::cpdata[Teapot::patchdata[patchNum]
              [u * 4 + (3 - v)]][ 2 ]
          );
        } else
        {
          patch[u][v] = glm::vec3(
            Teapot::cpdata[Teapot::patchdata[patchNum][u * 4 + v]][ 0 ],
            Teapot::cpdata[Teapot::patchdata[patchNum][u * 4 + v]][ 1 ],
            Teapot::cpdata[Teapot::patchdata[patchNum][u * 4 + v]][ 2 ]
          );
        }
      }
    }
  }
  
  std::shared_ptr< Buffer > vertexBuffer;
};

#endif /* __TEAPOT_QUAD__ */