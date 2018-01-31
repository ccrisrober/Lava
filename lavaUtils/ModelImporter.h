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

#ifndef __LAVAUTILS_MODELIMPORTER__
#define __LAVAUTILS_MODELIMPORTER__

#ifdef LAVA_USE_ASSIMP

#include <lavaUtils/api.h>

#include "Mesh.h"
#include "Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace lava
{
  namespace utility
  {
    class ModelImporter
    {
    public:
      LAVAUTILS_API
      ModelImporter( const std::string& path );
    public:
      std::vector< Mesh > _meshes;
      std::vector< Material > _materials;
    };
  }
}

#endif

#endif /* __LAVAUTILS_MODELIMPORTER__ */
