#include <lava/lava.h>

using namespace lava;

using namespace lava::engine;

class MeshBuilder
{
public:
  enum class Mode : short
  {
    Point, Lines, LineStrip,
    Triangles, TriangleStrip, TriangleFan
  };
  enum class Type
  {
    Indice,
    Position,
    Normal
  };
  struct Attribute
  {
    struct Buffer
    {
      std::shared_ptr<Buffer> buffer;
      uint32_t size = 0;
      uint32_t numElements = 0;
    } buffer;
  };
  void addAttributeBuffer( void* data, uint32_t typeSize, uint32_t length, Type type )
  {
    Attribute attribute;
    attribute.buffer.size = typeSize * length;
    attribute.buffer.numElements = length;
    //attribute.buffer.buffer = 

    _attributes.push_back( std::move( attribute ) );
  }
  void build( )
  {
    if ( builded )
    {
      return;
    }
    builded = true;
  }
  std::vector<Attribute> _attributes;
  Mode mode = Mode::Triangles;
  bool builded = false;
};

#ifndef M_PI
  #define M_PI 3.14159
#endif

int main( )
{
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<uint16_t> indices;

  // Generate positions / normals / indices
  {
    const int X_SEGMENTS = 64;
    const int Y_SEGMENTS = 64;
    for ( int y = 0; y <= Y_SEGMENTS; ++y )
    {
      for ( int x = 0; x <= X_SEGMENTS; ++x )
      {
        float xSegment = ( float ) x / ( float ) X_SEGMENTS;
        float ySegment = ( float ) y / ( float ) Y_SEGMENTS;
        float xPos = std::cos( xSegment * 2.0f * M_PI ) * std::sin( ySegment * M_PI );
        float yPos = std::cos( ySegment * M_PI );
        float zPos = std::sin( xSegment * 2.0f * M_PI ) * std::sin( ySegment * M_PI );

        positions.push_back( { xPos, yPos, zPos } );
        normals.push_back( { xPos, yPos, zPos } );
      }
    }

    bool oddRow = false;
    for ( int y = 0; y < Y_SEGMENTS; ++y )
    {
      if ( !oddRow )
      { // even rows: y == 0, y == 2; and so on
        for ( int x = 0; x <= X_SEGMENTS; ++x )
        {
          indices.push_back( static_cast< uint16_t >( ( y + 1 ) * ( X_SEGMENTS + 1 ) + x ) );
          indices.push_back( static_cast< uint16_t >( y         * ( X_SEGMENTS + 1 ) + x ) );
        }
      }
      else
      {
        for ( int x = X_SEGMENTS; x >= 0; --x )
        {
          indices.push_back( static_cast< uint16_t >( y         * ( X_SEGMENTS + 1 ) + x ) );
          indices.push_back( static_cast< uint16_t >( ( y + 1 ) * ( X_SEGMENTS + 1 ) + x ) );
        }
      }

      oddRow = !oddRow;
    }
  }

  MeshBuilder builder;
  builder.addAttributeBuffer( indices.data( ), sizeof( indices.front( ) ),
    indices.size( ), MeshBuilder::Type::Indice );
  builder.addAttributeBuffer( positions.data( ), sizeof( positions.front( ) ),
    positions.size( ), MeshBuilder::Type::Position );
  builder.addAttributeBuffer( normals.data( ), sizeof( normals.front( ) ),
    normals.size( ), MeshBuilder::Type::Normal );

  builder.build( );
  return 0;
}