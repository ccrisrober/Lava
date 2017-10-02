#ifndef __LAVA_ENGINE_MESH__
#define __LAVA_ENGINE_MESH__

#include <lava/api.h>

namespace lava
{
  namespace engine
  {
    class Mesh
    {
    public:
      class PrimitiveSet
      {
      public:
        struct Attribute
        {
          enum class Type : uint8_t
          {
            Indice,
            Position,
            Normal,
            TexCoord,
            Color,
          } type;
        };
        struct Buffer
        {
          char* data = nullptr;
          uint32_t size = 0;
          uint32_t elementsCount = 0;
        } buffer;

        void* data = nullptr;
      };
      class Builder
      {
      public:
        void build( void )
        {
          uint32_t i = 0;
          for( auto& att: _attributes )
          {
            if ( att.type == PrimitiveSet::Attribute::Type == 
              PrimitiveSet::Attribute::Indice )
            {
              // usage = vk::BufferUsage::eIndexBuffer;
            }
            else
            {
              // usage = vk::BufferUsage::eVertexBuffer;
            }
          }
        }
        void addAttributeBuffer( const void* data, uint32_t elementSize, 
          uint32_t elementsCount, PrimitiveSet::Attribute::Type type )
        {
          PrimitiveSet::Attribute att;
          att.type = type;
          att.buffer.size = elementSize * elementsCount;
          att.buffer.data = new char[ att.buffer.size ];
          att.buffer.elementsCount = elementsCount; 

          std::memcpy( att.buffer.data, static_cast<const char*>( data ), att.buffer.size );
          
          _attributes.push_back( std::move( att ) );
        }
      protected:
        std::vector< PrimitiveSet::Attribute > _attributes;
      };
    };
  }
}

#endif /* __LAVA_ENGINE_MESH__ */