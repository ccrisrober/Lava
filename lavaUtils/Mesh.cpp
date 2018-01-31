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
  }
}
#endif