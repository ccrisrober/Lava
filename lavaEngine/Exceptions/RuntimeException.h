#ifndef __LAVAENGINE_RUNTIME_EXCEPTION__
#define __LAVAENGINE_RUNTIME_EXCEPTION__

#include <lavaEngine/api.h>

#include "Exception.h"

namespace lava
{
	namespace engine
	{
		class RuntimeException: public Exception
		{
		public:
			LAVAENGINE_API
			RuntimeException( std::string message )
			: Exception( "PROGRAM TERMINATED BECAUSE OF AN ERROR: " + message )
			{
			}
		};
	}
}

#endif /* __LAVAENGINE_RUNTIME_EXCEPTION__ */
