#ifndef __LAVAENGINE_HAS_PARENT_EXCEPTION__
#define __LAVAENGINE_HAS_PARENT_EXCEPTION__

#include <lavaEngine/api.h>

#include "Exception.h"

namespace lava
{
  namespace engine
  {
    class HasParentException: public Exception
    {
    public:
        LAVAENGINE_API
      HasParentException( std::string childName,
        std::string parentName, std::string targetName )
        : Exception( "Cannot attach node (\"" + childName +
          "\") to (\"" + targetName +
          "\") because it already has a parent (\"" +
          parentName + "\")" )
      {
      }
    };
  }
}

#endif /* __LAVAENGINE_HAS_PARENT_EXCEPTION__ */
