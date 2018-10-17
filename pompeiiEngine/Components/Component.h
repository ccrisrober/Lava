/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#ifndef __POMPEIIENGINE_COMPONENT__
#define __POMPEIIENGINE_COMPONENT__

#include <iostream>
#include <pompeiiEngine/Clock.h>
#include <pompeiiEngine/api.h>

namespace pompeii
{
  namespace engine
  {
  }
}

namespace pompeii
{
  namespace engine
  {
    typedef std::string ComponentUID;
    #define IMPLEMENT_COMPONENT(__CLASS__) \
    public: \
      static pompeii::engine::ComponentUID StaticGetUID( void ) { \
      static std::string sUID = #__CLASS__; \
      return ( pompeii::engine::ComponentUID ) sUID; /* This will be unique! */ \
      } \
      virtual pompeii::engine::ComponentUID GetUID( void ) const{ return StaticGetUID( ); }

    class Node;

    class Component
    {
      friend class Node;
    public:
      POMPEIIENGINE_API
      virtual ComponentUID GetUID( void ) const = 0;
      POMPEIIENGINE_API
      virtual ~Component( void );
      POMPEIIENGINE_API
      Node* node( void );
      POMPEIIENGINE_API
      const Node* getNode( void ) const
      {
        return _node;
      }
      POMPEIIENGINE_API
      virtual void update( const pompeii::engine::Clock& clock );
      POMPEIIENGINE_API
      virtual void start( void );
      POMPEIIENGINE_API
      // Invoked once when component is attached to a node
      virtual void onAttach( void );
      POMPEIIENGINE_API
      // Invoked once when component is detached from a node
      virtual void onDetach( void );
      POMPEIIENGINE_API
      bool isEnabled( void ) const;
      POMPEIIENGINE_API
      void enable( void );
      POMPEIIENGINE_API
      void disable( void );
      POMPEIIENGINE_API
      void setEnabled( const bool v );
      POMPEIIENGINE_API
      void toggle( void );
      POMPEIIENGINE_API
      virtual void onEnable( void ) { }
      POMPEIIENGINE_API
      virtual void onDisable( void ) { }

      // Unuse
      // TODO: virtual void onRenderDebug( Renderer*, Camera* ) { }
    private:
      void setNode( Node* n );
    protected:
      bool _enabled;
      POMPEIIENGINE_API
      Component( void );
      Node* _node;
    };
  }
}

#endif /* __POMPEIIENGINE_COMPONENT__ */
