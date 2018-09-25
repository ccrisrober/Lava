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

#ifndef __POMPEII_ENGINE_SWITCH__
#define __POMPEII_ENGINE_SWITCH__

#include "Group.h"
#include <pompeiiEngine/api.h>

namespace pompeii
{
  namespace engine
  {
    class Switch : public Group
    {
    public:
      POMPEIIENGINE_API
      Switch( const std::string& name );
      POMPEIIENGINE_API
      virtual ~Switch( void );

      POMPEIIENGINE_API
      virtual void forEachNode( std::function<void( Node* )> cb );
      POMPEIIENGINE_API
      Node* currentNode( void );

      POMPEIIENGINE_API
      unsigned int getActiveChild( void ) const;
      POMPEIIENGINE_API
      void setActiveChild( unsigned int idx );
      POMPEIIENGINE_API
      void disableAllChildren( void )
      {
        _activeChild = SW_INVALID_CHILD;
      }

      POMPEIIENGINE_API
      void selectNextNode( void );
      POMPEIIENGINE_API
      void selectPreviousNode( void );

      enum { SW_INVALID_CHILD = -1 };
    private:
      int _activeChild;
    };
  }
}

#endif /* __POMPEII_ENGINE_SWITCH__ */