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

#ifndef __LAVAENGINE_VISITOR__
#define __LAVAENGINE_VISITOR__

#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class Node;
    class Group;
    class Light;
    class Camera;
    class Geometry;

    class Visitor
    {
    protected:
      Visitor( void ) { }
    public:
      LAVAENGINE_API
      virtual ~Visitor( void );
      LAVAENGINE_API
      virtual void reset( void );
      LAVAENGINE_API
      virtual void traverse( Node* node );
      LAVAENGINE_API
      virtual void visitNode( Node *node );
      LAVAENGINE_API
      virtual void visitGroup( Group *group );
      LAVAENGINE_API
      virtual void visitCamera( Camera *camera );
      LAVAENGINE_API
      virtual void visitLight( Light* light );
      LAVAENGINE_API
      virtual void visitGeometry( Geometry *geometry );
    private:
      Visitor( const Visitor& );
      Visitor &operator= ( const Visitor& );
    };
  }
}

#endif /* __LAVAENGINE_VISITOR__ */
