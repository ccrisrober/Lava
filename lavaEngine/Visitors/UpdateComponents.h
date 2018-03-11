#ifndef __LAVAENGINE_UPDATE_COMPONENTS__
#define __LAVAENGINE_UPDATE_COMPONENTS__

#include "LambdaVisitor.h"
#include <lavaEngine/api.h>

#include <lavaEngine/Clock.h>

namespace lava
{
	namespace engine
	{
	  class UpdateComponents :
	    public LambdaVisitor
	  {
	  public:
	    LAVAENGINE_API
	    UpdateComponents( const lava::engine::Clock& clock );
	  };
	}
}

#endif /* __LAVAENGINE_UPDATE_COMPONENTS__ */
