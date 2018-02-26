
#ifndef __MB_LAMBDA_COMPONENT__
#define __MB_LAMBDA_COMPONENT__

#include "Component.hpp"
#include <mb/api.h>
#include <functional>

namespace mb
{
  typedef std::function< void( mb::Node*, const mb::Clock& clock ) > LambdaCallback;
  class LambdaComponent : public Component
  {
    IMPLEMENT_COMPONENT( LambdaComponent )
  public:
    MB_API
    LambdaComponent( LambdaCallback cb );
    MB_API
    virtual void update( const mb::Clock& clock ) override;
  protected:
    LambdaCallback _callback;
  };
}

#endif /* __MB_LAMBDA_COMPONENT__ */
