#include "DumpVisitor.h"
#include <iostream>
#include <sstream>

#include <lavaEngine/Scenegraph/Node.h>
#include <lavaEngine/Scenegraph/Group.h>
//#include <lavaEngine/Scenegraph/Geometry.h>
//#include <lavaEngine/Scenegraph/Camera.h>
#include <lavaEngine/Scenegraph/Light.h>

namespace lava
{
  namespace engine
  {
    void DumpVisitor::traverse( Node *node )
    {
      std::cout << "TRAVERSING ... " << std::endl;

      Visitor::traverse( node );

      std::cout << std::endl << "... DONE" << std::endl;
    }

    void DumpVisitor::visitNode( Node *node )
    {
      Visitor::visitNode( node );

      _dumpNode( node, "Node" );
    }

    void DumpVisitor::visitGroup( Group *group )
    {
      _dumpNode( group, "Group" );

      ++_auxLevel;
      Visitor::visitGroup( group );
      --_auxLevel;
    }

    /*void DumpVisitor::visitGeometry( Geometry *geometry )
    {
      _dumpNode( geometry, "Geometry" );
    }

    void DumpVisitor::visitCamera( Camera *camera )
    {
      _dumpNode( camera, "Camera" );
    }*/

    void DumpVisitor::visitLight( Light *light )
    {
      _dumpNode( light, "Light" );
    }

    void DumpVisitor::_dumpNode( Node *node, const std::string& type )
    {
      std::cout << "\n";
      std::stringstream tab;
      for ( unsigned int i = 0; i < _auxLevel; ++i )
      {
        tab << "  ";
      }

      std::cout << tab.str() << "[" + type + "] "
        << node->name() << " (" << node << ")";
    }
  }
}
