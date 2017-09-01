#include "Mesh.h"

#ifdef LAVA_USE_ASSIMP
namespace lava
{
  namespace extras
  {
    Mesh::Mesh( const aiMesh *mesh )
    {
      for ( uint32_t i = 0; i < mesh->mNumVertices; ++i )
      {
        Vertex v;

        v.position = glm::vec3(
          mesh->mVertices[ i ].x,
          mesh->mVertices[ i ].y,
          mesh->mVertices[ i ].z );
        v.normal = glm::vec3(
          mesh->mNormals[ i ].x,
          mesh->mNormals[ i ].y,
          mesh->mNormals[ i ].z );

        if ( mesh->HasTextureCoords( 0 ) )
          v.texCoord = glm::vec2(
            mesh->mTextureCoords[ 0 ][ i ].x,
            mesh->mTextureCoords[ 0 ][ i ].y );

        vertices.emplace_back( v );
      }

      for ( uint32_t i = 0; i < mesh->mNumFaces; ++i )
      {
        for ( uint32_t j = 0; j < 3; ++j )
        {
          indices.emplace_back( mesh->mFaces[ i ].mIndices[ i ] );
        }
      }

      numVertices = mesh->mNumVertices;
      numIndices = mesh->mNumFaces * 3;
    }
  }
}
#endif