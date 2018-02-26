#ifndef __LAVAENGINE_LAMBDA_VISITOR__
#define __LAVAENGINE_LAMBDA_VISITOR__

#include <functional>
#include "Visitor.h"
#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class LambdaVisitor :
      public Visitor
    {
    private:
      typedef std::function< void( Node* ) > CallbackType;
    public:
      LAVAENGINE_API
      LambdaVisitor( CallbackType cb );
      LAVAENGINE_API
      virtual void visitNode( Node* n ) override;
      LAVAENGINE_API
      virtual void visitGroup( Group* g ) override;
    private:
      CallbackType _callback;
    };
  }
}

#endif /* __LAVAENGINE_LAMBDA_VISITOR__ */
