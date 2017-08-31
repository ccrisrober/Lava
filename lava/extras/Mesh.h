#ifndef __LAVA_MESH__
#define __LAVA_MESH__

#include <vector>
#include <glm/glm.hpp>
#include <assimp/mesh.h>

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
    struct Mesh
    {
      Mesh( const aiMesh *mesh );
      uint32_t numVertices;
      uint32_t numIndices;
      std::vector< Vertex > vertices;
      std::vector< uint32_t > indices;
    };
  }
}

#endif /* __LAVA_MESH__ */