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

#ifndef __POMPEIIENGINE_CHILDREN_COUNTER_VISITOR__
#define __POMPEIIENGINE_CHILDREN_COUNTER_VISITOR__

#include "Visitor.h"
#include <pompeiiEngine/api.h>

namespace pompeii
{
  namespace engine
  {
    class ChildrenCounterVisitor :
      public Visitor
    {
    public:
      POMPEIIENGINE_API
      ChildrenCounterVisitor( void );
      POMPEIIENGINE_API
      virtual void reset( void ) override;
      POMPEIIENGINE_API
      virtual void traverse( Node* n ) override;
      POMPEIIENGINE_API
      virtual void visitNode( Node* n ) override;
      POMPEIIENGINE_API
      virtual void visitGroup( Group* g ) override;

    //protected:
      unsigned int _childrens;
    };
  }
}

#endif /* __POMPEIIENGINE_CHILDREN_COUNTER_VISITOR__ */
