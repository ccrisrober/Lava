#include "PrintNameVisitor.h"
#include <iostream>
#include <string>
#include <lavaEngine/Scenegraph/Node.h>

namespace lava
{
  namespace engine
  {
    PrintNameVisitor::PrintNameVisitor( void )
      : LambdaVisitor( [ ]( Node*n )
        {
          std::cout << n->name( ) << std::endl;
        } )
    {
    }
  }
}
