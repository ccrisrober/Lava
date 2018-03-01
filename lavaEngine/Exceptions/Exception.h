#ifndef __LAVAENGINE_EXCEPTION__
#define __LAVAENGINE_EXCEPTION__

#include <lavaEngine/api.h>

#include <stdexcept>
#include <string>

namespace lava
{
	namespace engine
	{
	  class Exception: public std::runtime_error
	  {
	  public:
	      LAVAENGINE_API
	    Exception( std::string message )
	    : std::runtime_error( message.c_str( ) )
	    {
	    }
	  };
	}
}

#endif /* __LAVAENGINE_EXCEPTION__ */
