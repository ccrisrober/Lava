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
