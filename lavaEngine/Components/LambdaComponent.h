#ifndef __LAVAENGINE_LAMBDA_COMPONENT__
#define __LAVAENGINE_LAMBDA_COMPONENT__

#include "Component.h"
#include <lavaEngine/api.h>
#include <functional>

namespace lava
{
  namespace engine
  {
    typedef std::function< void( mb::Node*, const /*mb::Clock&*/float clock ) > LambdaCallback;
    class LambdaComponent
      : public Component
    {
      IMPLEMENT_COMPONENT( LambdaComponent )
    public:
      LAVAENGINE_API
      LambdaComponent( LambdaCallback cb );
      //MB_API
      //virtual void update( const mb::Clock& clock ) override;
    protected:
      LambdaCallback _callback;
    };
  }
}

#endif /* __LAVAENGINE_LAMBDA_COMPONENT__ */
