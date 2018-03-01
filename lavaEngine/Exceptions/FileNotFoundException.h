#ifndef __LAVAENGINE_FILE_NOT_FOUND_EXCEPTION__
#define __LAVAENGINE_FILE_NOT_FOUND_EXCEPTION__

#include <lavaEngine/api.h>

#include "Exception.h"

namespace lava
{
	namespace engine
	{
	  class FileNotFoundException: public Exception
	  {
	  public:
	    LAVAENGINE_API
	    FileNotFoundException( std::string filePath )
	    : Exception( "Cannot find file " + filePath )
	    {
	    }
	  };
	}
}

#endif /* __LAVAENGINE_FILE_NOT_FOUND_EXCEPTION__ */
