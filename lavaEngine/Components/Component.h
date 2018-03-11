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

#ifndef __LAVAENGINE_COMPONENT__
#define __LAVAENGINE_COMPONENT__

#include <iostream>
#include <lavaEngine/Clock.h>
#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    //class Renderer;
    class Camera;
  }
}

namespace lava
{
  namespace engine
  {
    typedef std::string ComponentUID;
    #define IMPLEMENT_COMPONENT(__CLASS__) \
    public: \
      static lava::engine::ComponentUID StaticGetUID( void ) { \
      static std::string sUID = #__CLASS__; \
      return ( lava::engine::ComponentUID ) sUID; /* This will be unique! */ \
      } \
      virtual lava::engine::ComponentUID GetUID( void ) const{ return StaticGetUID( ); }

    class Node;

    class Component
    {
      friend class Node;
    public:
      LAVAENGINE_API
      virtual ComponentUID GetUID( void ) const = 0;
      LAVAENGINE_API
      virtual ~Component( void );
      LAVAENGINE_API
      Node* node( void );
      LAVAENGINE_API
      const Node* getNode( void ) const
      {
        return _node;
      }
      LAVAENGINE_API
      virtual void update( const lava::engine::Clock& clock );
      LAVAENGINE_API
      virtual void start( void );
      LAVAENGINE_API
      // Invoked once when component is attached to a node
      virtual void onAttach( void );
      LAVAENGINE_API
      // Invoked once when component is detached from a node
      virtual void onDetach( void );
      LAVAENGINE_API
      bool isEnabled( void ) const;
      LAVAENGINE_API
      void enable( void );
      LAVAENGINE_API
      void disable( void );
      LAVAENGINE_API
      void setEnabled( const bool v );
      LAVAENGINE_API
      void toggle( void );
      LAVAENGINE_API
      virtual void onEnable( void ) { }
      LAVAENGINE_API
      virtual void onDisable( void ) { }

      // Unuse
      // TODO: virtual void onRenderDebug( Renderer*, Camera* ) { }
    private:
      void setNode( Node* n );
    protected:
      bool _enabled;
      LAVAENGINE_API
      Component( void );
      Node* _node;
    };
  }
}

#endif /* __LAVAENGINE_COMPONENT__ */
