#ifndef __LAVAENGINE_PRINT_NAME_VISITOR__
#define __LAVAENGINE_PRINT_NAME_VISITOR__

#include "LambdaVisitor.h"
#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class PrintNameVisitor :
      public LambdaVisitor
    {
    public:
      LAVAENGINE_API
        PrintNameVisitor( void );
    };
  }
}

#endif /* __LAVAENGINE_PRINT_NAME_VISITOR__ */
