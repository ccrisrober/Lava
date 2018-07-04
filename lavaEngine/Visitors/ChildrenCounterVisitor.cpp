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
