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

#include "ModelImporter.h"

#include <iostream>

#include <lava/lava.h>

#ifdef LAVA_USE_ASSIMP

namespace lava
{
  namespace utility
  {
    ModelImporter::ModelImporter( const std::string& path )
    {
      Assimp::Importer imp;

      std::string globalPath;
      std::size_t last = path.find_last_of( '/' );

      if ( last != std::string::npos )
      {
        globalPath = path;
        globalPath.erase( last );
        globalPath += "/";
      }
      else
      {
        globalPath = "./";
      }

      aiScene const *scene = imp.ReadFile( path,
        aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs );

      if ( scene == nullptr )
      {
        //Log::error( "Mesh %s undefined", path );
        throw std::runtime_error( path + " doesn't opened" );
      }

      for ( uint32_t i = 0; i < scene->mNumMeshes; ++i )
      {
        _meshes.emplace_back( scene->mMeshes[ i ] );
      }

      for ( uint32_t i = 0; i < scene->mNumMaterials; ++i )
      {
        _materials.emplace_back( scene->mMaterials[ i ], globalPath );
      }
    }
  }
}

#endif
