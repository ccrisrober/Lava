#ifndef __LAVAENGINE_CHILDREN_COUNTER_VISITOR__
#define __LAVAENGINE_CHILDREN_COUNTER_VISITOR__

#include "Visitor.h"
#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class ChildrenCounterVisitor :
      public Visitor
    {
    public:
      LAVAENGINE_API
      ChildrenCounterVisitor( void );
      LAVAENGINE_API
      virtual void reset( void ) override;
      LAVAENGINE_API
      virtual void traverse( Node* n ) override;
      LAVAENGINE_API
      virtual void visitNode( Node* n ) override;
      LAVAENGINE_API
      virtual void visitGroup( Group* g ) override;

    //protected:
      unsigned int _childrens;
    };
  }
}

#endif /* __LAVAENGINE_CHILDREN_COUNTER_VISITOR__ */
