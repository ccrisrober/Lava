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

#ifndef __POMPEIIENGINE_SCENE__
#define __POMPEIIENGINE_SCENE__

#include "Node.h"

namespace pompeii
{
  namespace engine
  {
    class Scene
    {
    public:
      POMPEIIENGINE_API
      Scene( Node* root );
      POMPEIIENGINE_API
      Node* getRoot( void ) const;
      POMPEIIENGINE_API
      void setRoot( Node* root );
    protected:
      Node* _root;
    };
  }
}

#endif /* __POMPEIIENGINE_SCENE__ */