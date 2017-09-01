#ifndef __LAVA_MATERIAL__
#define __LAVA_MATERIAL__

#ifdef LAVA_USE_ASSIMP

#include <assimp/material.h>
#include "../includes.hpp"

namespace lava
{
  namespace extras
  {
    struct Material
    {
      Material( const aiMaterial* mtl, const std::string& globalPath );

      std::string albedoTexturePath;
      glm::vec3 albedoColor;
      bool useAlbedoTexture;
    };
  }
}

#endif

#endif /* __LAVA_MATERIAL__  */