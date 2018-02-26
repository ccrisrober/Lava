#ifndef __LAVAENGINE_FIND_NODES__
#define __LAVAENGINE_FIND_NODES__

#include "Visitor.h"
#include <functional>
#include <vector>

#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class FindNodes: public Visitor
    {
    public:
      using NodeSelector = std::function< bool( Node* ) >;
    public:
      LAVAENGINE_API
      FindNodes( NodeSelector sel );
      LAVAENGINE_API
      virtual void eachMatchNodes( std::function< void( Node* ) > cb );
      LAVAENGINE_API
      virtual void reset( void ) override;
      LAVAENGINE_API
      virtual void visitNode( Node* n ) override;
      LAVAENGINE_API
      virtual void visitGroup( Group* g ) override;
      LAVAENGINE_API
      std::vector< Node* > matches( void ) const
      {
        return _matches;
      }
    protected:
      NodeSelector _selector;
      std::vector< Node * > _matches;
    };
  }
}

#endif /* __LAVAENGINE_FIND_NODES__ */
