#include "ModelImporter.h"

#ifdef LAVA_USE_ASSIMP

namespace lava
{
  namespace extras
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
        throw std::runtime_error( path + " does not opened" );
      }

      for ( uint32_t i = 0; i < scene->mNumMeshes; ++i )
      {
        _meshes.emplace_back( scene->mMeshes[ i ] );
      }

      for ( uint32_t i = 0; i < scene->mNumMaterials; ++i )
        _materials.emplace_back( scene->mMaterials[ i ], globalPath );
    }
  }
}

#endif