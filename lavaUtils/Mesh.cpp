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

#include "Mesh.h"

#ifdef LAVA_USE_ASSIMP
namespace lava
{
  namespace utility
  {
    Mesh::Mesh( const aiMesh *mesh )
    {
      for ( uint32_t i = 0; i < mesh->mNumVertices; ++i )
      {
        Vertex v;

        v.position = glm::vec3(
          mesh->mVertices[ i ].x,
          mesh->mVertices[ i ].y,
          mesh->mVertices[ i ].z );
        v.normal = glm::vec3(
          mesh->mNormals[ i ].x,
          mesh->mNormals[ i ].y,
          mesh->mNormals[ i ].z );

        if ( mesh->HasTextureCoords( 0 ) )
        {
          v.texCoord = glm::vec2(
            mesh->mTextureCoords[ 0 ][ i ].x,
            mesh->mTextureCoords[ 0 ][ i ].y );
        }
        else
        {
          v.texCoord = glm::vec2( 0.0f );
        }

        vertices.push_back( v );
      }

      /*for ( uint32_t i = 0; i < mesh->mNumFaces; ++i )
      {
        for ( uint32_t j = 0; j < 3; ++j )
        {
          indices.emplace_back( mesh->mFaces[ i ].mIndices[ i ] );
        }
      }*/
      for(uint32_t  i = 0; i < mesh->mNumFaces; ++i )
      {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for( uint32_t j = 0; j < face.mNumIndices; ++j )
          indices.push_back(face.mIndices[j]);
      }

      numVertices = mesh->mNumVertices;
      numIndices = mesh->mNumFaces * 3;
    }
    void Mesh::convertFacesToAdjancencyFormat( void )
    {
      // Elements with adjacency info
      std::vector< uint32_t > elAdj( indices.size( ) * 2 );

      // Copy and make room for adjacency info
      for( uint32_t i = 0; i < indices.size( ); i+= 3 )
      {
        elAdj[i*2 + 0] = indices[i];
        elAdj[i*2 + 1] = std::numeric_limits< uint32_t >::max( );
        elAdj[i*2 + 2] = indices[i+1];
        elAdj[i*2 + 3] = std::numeric_limits< uint32_t >::max( );
        elAdj[i*2 + 4] = indices[i+2];
        elAdj[i*2 + 5] = std::numeric_limits< uint32_t >::max( );
      }

      // Find matching edges
      for( uint32_t i = 0; i < elAdj.size( ); i+=6)
      {
        // A triangle
        uint32_t a1 = elAdj[i];
        uint32_t b1 = elAdj[i+2];
        uint32_t c1 = elAdj[i+4];

        // Scan subsequent triangles
        for(uint32_t j = i+6; j < elAdj.size(); j+=6)
        {
          uint32_t a2 = elAdj[j];
          uint32_t b2 = elAdj[j+2];
          uint32_t c2 = elAdj[j+4];

          // Edge 1 == Edge 1
          if( (a1 == a2 && b1 == b2) || (a1 == b2 && b1 == a2) )
          {
            elAdj[i+1] = c2;
            elAdj[j+1] = c1;
          }
          // Edge 1 == Edge 2
          if( (a1 == b2 && b1 == c2) || (a1 == c2 && b1 == b2) )
          {
            elAdj[i+1] = a2;
            elAdj[j+3] = c1;
          }
          // Edge 1 == Edge 3
          if ( (a1 == c2 && b1 == a2) || (a1 == a2 && b1 == c2) )
          {
            elAdj[i+1] = b2;
            elAdj[j+5] = c1;
          }
          // Edge 2 == Edge 1
          if( (b1 == a2 && c1 == b2) || (b1 == b2 && c1 == a2) )
          {
            elAdj[i+3] = c2;
            elAdj[j+1] = a1;
          }
          // Edge 2 == Edge 2
          if( (b1 == b2 && c1 == c2) || (b1 == c2 && c1 == b2) )
          {
            elAdj[i+3] = a2;
            elAdj[j+3] = a1;
          }
          // Edge 2 == Edge 3
          if( (b1 == c2 && c1 == a2) || (b1 == a2 && c1 == c2) )
          {
            elAdj[i+3] = b2;
            elAdj[j+5] = a1;
          }
          // Edge 3 == Edge 1
          if( (c1 == a2 && a1 == b2) || (c1 == b2 && a1 == a2) )
          {
            elAdj[i+5] = c2;
            elAdj[j+1] = b1;
          }
          // Edge 3 == Edge 2
          if( (c1 == b2 && a1 == c2) || (c1 == c2 && a1 == b2) )
          {
            elAdj[i+5] = a2;
            elAdj[j+3] = b1;
          }
          // Edge 3 == Edge 3
          if( (c1 == c2 && a1 == a2) || (c1 == a2 && a1 == c2) )
          {
            elAdj[i+5] = b2;
            elAdj[j+5] = b1;
          }
          }
      }

      // Look for any outside edges
      for( uint32_t i = 0; i < elAdj.size( ); i+=6)
      {
        if( elAdj[ i + 1 ] == std::numeric_limits< uint32_t >::max( ) ) elAdj[ i + 1 ] = elAdj[i+4];
        if( elAdj[ i + 3 ] == std::numeric_limits< uint32_t >::max( ) ) elAdj[ i + 3 ] = elAdj[i];
        if( elAdj[ i + 5 ] == std::numeric_limits< uint32_t >::max( ) ) elAdj[ i + 5 ] = elAdj[i+2];
      }

      // Copy all data back into el
      indices = elAdj;


      numIndices = numIndices * 2;
    }
  }
}
#endif
