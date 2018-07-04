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

#include "Material.h"
#include <iostream>

#ifdef LAVA_USE_ASSIMP

namespace lava
{
  namespace utility
  {
    Material::Material( const aiMaterial* mtl, const std::string& globalPath )
    {
      aiString texPath;

      if ( mtl->GetTexture( aiTextureType_DIFFUSE, 0, &texPath ) == AI_SUCCESS )
      {
        std::string path = texPath.C_Str( );

        if ( path[ 0 ] == '.' )
        {
          path.erase( 0, 1 );
        }

        if ( path[ 0 ] == '/' || path[ 0 ] == '\\' )
        {
          path.erase( 0, 1 );
        }

        for ( auto &v : path )
        {
          if ( v == '\\' )
          {
            v = '/';
          }
        }

        albedoTexturePath = globalPath + path;
        useAlbedoTexture = true;
      }

      else
      {
        aiColor3D color;

        if ( mtl->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS )
        {
          this->albedoColor = glm::vec3( color.r, color.g, color.b );
        }

        else
        {
          this->albedoColor = glm::vec3( 1.0, 1.0, 1.0 );
        }
        useAlbedoTexture = false;
      }
    }
  }
}

#endif
