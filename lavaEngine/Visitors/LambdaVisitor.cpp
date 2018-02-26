#include "LambdaVisitor.h"
#include <lavaEngine/Scenegraph/Node.h>
#include <lavaEngine/Scenegraph/Group.h>

namespace lava
{
  namespace engine
  {
    LambdaVisitor::LambdaVisitor( CallbackType cb )
      : _callback( cb )
    {
    }

    void LambdaVisitor::visitNode( Node* n )
    {
      _callback( n );
    }

    void LambdaVisitor::visitGroup( Group *group )
    {
      _callback( group );
      Visitor::visitGroup( group );
    }
  }
}
