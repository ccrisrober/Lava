#ifndef __LAVA_MESH__
#define __LAVA_MESH__

#ifdef LAVA_USE_ASSIMP

#include <vector>
#include <glm/glm.hpp>
#include <assimp/mesh.h>

#include <lava/api.h>

namespace lava
{
  namespace extras
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
      LAVA_API
      Mesh( const aiMesh *mesh );
    public:
      uint32_t numVertices;
      uint32_t numIndices;
      std::vector< Vertex > vertices;
      std::vector< uint32_t > indices;
    };
  }
}

#endif

#endif /* __LAVA_MESH__ */