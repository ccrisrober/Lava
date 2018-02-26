#ifndef __LAVANENGINE_FETCH_LIGHTS__
#define __LAVANENGINE_FETCH_LIGHTS__

#include "Visitor.h"
#include <vector>
#include <functional>

#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class FetchLights :
      public Visitor
    {
    public:
      LAVAENGINE_API
      virtual void reset( void ) override;
      LAVAENGINE_API
      virtual void visitLight( Light* c ) override;
      LAVAENGINE_API
      bool hasLights( void ) const
      {
        return !_lights.empty( );
      }
      LAVAENGINE_API
      std::vector<Light*> lights( void ) const
      {
        return _lights;
      }
      LAVAENGINE_API
      void forEachLight( std::function<void( Light* )> cb );
    protected:
      std::vector<Light*> _lights;
    };
  }
}

#endif /* __LAVANENGINE_FETCH_LIGHTS__ */
