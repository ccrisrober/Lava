#ifndef __LAVA_MODELIMPORTER__
#define __LAVA_MODELIMPORTER__

#ifdef LAVA_USE_ASSIMP

#include "Mesh.h"
#include "Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace lava
{
  namespace extras
  {
    class ModelImporter
    {
    public:
      LAVA_API
      ModelImporter( const std::string& path );
    public:
      std::vector< Mesh > _meshes;
      std::vector< Material > _materials;
    };
  }
}

#endif

#endif /* __LAVA_MODELIMPORTER__ */