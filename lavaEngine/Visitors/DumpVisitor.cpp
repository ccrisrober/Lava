/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "DumpVisitor.h"
#include <iostream>
#include <sstream>

#include <lavaEngine/Scenegraph/Node.h>
#include <lavaEngine/Scenegraph/Group.h>
#include <lavaEngine/Scenegraph/Geometry.h>
#include <lavaEngine/Scenegraph/Camera.h>
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

    void DumpVisitor::visitGeometry( Geometry *geometry )
    {
      _dumpNode( geometry, "Geometry" );
    }

    void DumpVisitor::visitCamera( Camera *camera )
    {
      _dumpNode( camera, "Camera" );
    }

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

      std::cout << tab.str( ) << "[" + type + "] "
        << node->name( ) << " (" << node << ")";
    }
  }
}
