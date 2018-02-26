#ifndef __LAVAENGINE_SCENE__
#define __LAVAENGINE_SCENE__

#include "Node.h"

namespace lava
{
  namespace engine
  {
    class Scene
    {
    public:
      LAVAENGINE_API
      Scene( Node* root );
      LAVAENGINE_API
      Node* getRoot( void ) const;
      LAVAENGINE_API
      void setRoot( Node* root );
    protected:
      Node* _root;
    };
  }
}

#endif /* __LAVAENGINE_SCENE__ */