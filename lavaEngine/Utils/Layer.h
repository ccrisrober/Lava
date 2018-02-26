#ifndef __LAVAENGINE_LAYER__
#define __LAVAENGINE_LAYER__

#include <lavaEngine/api.h>

#include <array>

namespace lava
{
  namespace engine
  {
    class LayerLookup
    {
    public:
      LAVAENGINE_API
      static void loadFromFile( const std::string& file );
      LAVAENGINE_API
      static const std::string getName( const int channel );
      LAVAENGINE_API
      static void setName( const std::string& name, const int index );
      LAVAENGINE_API
      static int nameToID( const std::string& layer );
    protected:
      static std::array<std::string, 32> _names;
      static bool _initialized;

      static void initialize( );
    };
    class Layer
    {
    public:
      LAVAENGINE_API
      Layer( void );
      LAVAENGINE_API
      void set( const int channel );
      LAVAENGINE_API
      void set( const std::string& layer );
      LAVAENGINE_API
      void enable( const int channel );
      LAVAENGINE_API
      void enable( const std::string& layer );
      LAVAENGINE_API
      void toggle( const int channel );
      LAVAENGINE_API
      void toggle( const std::string& layer );
      LAVAENGINE_API
      void disable( const int channel );
      LAVAENGINE_API
      void disable( const std::string& layer );
      LAVAENGINE_API
      bool check( const Layer& layer2 ) const;
      LAVAENGINE_API
      bool check( const int channel ) const;
      LAVAENGINE_API
      bool check( const std::string& layer ) const;
    protected:
      int _mask;
    private:
      static int layerNameToID( const std::string& layer );
    };
  }
}

#endif /* __LAVAENGINE_LAYER__ */
