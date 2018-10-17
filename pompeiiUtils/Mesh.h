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

#ifndef __POMPEIIUTILS_MESH__
#define __POMPEIIUTILS_MESH__

#ifdef POMPEII_USE_ASSIMP

#include <vector>
#include <glm/glm.hpp>
#include <assimp/mesh.h>

#include <pompeiiUtils/api.h>

namespace pompeii
{
  namespace utils
  {
    struct Vertex
    {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 texCoord;
    };
    class Mesh
    {
    public:
      POMPEIIUTILS_API
      Mesh( const aiMesh *mesh );

      POMPEIIUTILS_API
      void convertFacesToAdjancencyFormat( void );
    public:
      uint32_t numVertices;
      uint32_t numIndices;
      std::vector< Vertex > vertices;
      std::vector< uint32_t > indices;
    };
  }
}

#endif

#endif /* __POMPEIIUTILS_MESH__ */
