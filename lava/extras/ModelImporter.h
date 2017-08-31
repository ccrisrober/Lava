#ifndef __LAVA_MODELIMPORTER__
#define __LAVA_MODELIMPORTER__

#include "Mesh.h"

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
      ModelImporter( const std::string& path )
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
          throw std::runtime_error( path + " does not opened" );
        }

        for ( uint32_t i = 0; i < scene->mNumMeshes; ++i )
        {
          _meshes.emplace_back( scene->mMeshes[ i ] );
        }
      }
    public:
      std::vector< Mesh > _meshes;
    };
  }
}

#endif /* __LAVA_MODELIMPORTER__ */