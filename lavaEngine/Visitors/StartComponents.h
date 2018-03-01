#ifndef __LAVAENGINE_START_COMPONENTS__
#define __LAVAENGINE_START_COMPONENTS__

#include "LambdaVisitor.h"
#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
  	class StartComponents :
    public LambdaVisitor
	  {
	  public:
	    LAVAENGINE_API
	    StartComponents( void );
	  };
  }
}

#endif /* __LAVAENGINE_START_COMPONENTS__ */
