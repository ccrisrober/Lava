#include "ChildrenCounterVisitor.h"
#include <lavaEngine/Scenegraph/Node.h>
#include <lavaEngine/Scenegraph/Group.h>

#include <iostream>

namespace lava
{
  namespace engine
  {
    ChildrenCounterVisitor::ChildrenCounterVisitor( void )
    : Visitor( )
    , _childrens( 0 )
    {
    }

    void ChildrenCounterVisitor::reset( void )
    {
      _childrens = 0;
      Visitor::reset( );
    }

    void ChildrenCounterVisitor::traverse( Node* n )
    {
      this->reset( );
      Visitor::traverse( n );
      --_childrens;
    }
    void ChildrenCounterVisitor::visitNode( Node* n )
    {
      ++_childrens;
      std::cout << n->name( ) << std::endl;
    }

    void ChildrenCounterVisitor::visitGroup( Group* group )
    {
      visitNode( group );
      Visitor::visitGroup( group );
    }
  }
}
