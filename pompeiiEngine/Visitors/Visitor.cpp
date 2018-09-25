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

#include "Visitor.h"

#include <pompeiiEngine/Scenegraph/Node.h>
#include <pompeiiEngine/Scenegraph/Group.h>
#include <pompeiiEngine/Scenegraph/Geometry.h>
#include <pompeiiEngine/Scenegraph/Camera.h>
#include <pompeiiEngine/Scenegraph/Light.h>

namespace pompeii
{
  namespace engine
  {
    Visitor::~Visitor( void )
    {
    }

    Visitor::Visitor( const Visitor& )
    {
    }

    Visitor &Visitor::operator= ( const Visitor& )
    {
      return *this;
    }

    void Visitor::reset( void )
    {
    }

    void Visitor::traverse( Node* n )
    {
      reset( );
      n->accept( *this );
    }

    void Visitor::visitNode( Node* )
    {
    }

    void Visitor::visitGroup( Group* group )
    {
      group->forEachNode( [ & ]( Node* n )
      {
        n->accept( *this );
      } );
    }

    void Visitor::visitLight( Light* light )
    {
      visitNode( light );
    }

    void Visitor::visitCamera( Camera *camera )
    {
      visitNode( camera );
    }

    void Visitor::visitGeometry( Geometry *geometry )
    {
      visitNode( geometry );
    }
  }
}
