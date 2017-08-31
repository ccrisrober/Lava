#ifndef __LAVA_MODELIMPORTER__
#define __LAVA_MODELIMPORTER__

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
      ModelImporter( const std::string& path );
    public:
      std::vector< Mesh > _meshes;
      std::vector< Material > _materials;
    };
  }
}

#endif /* __LAVA_MODELIMPORTER__ */