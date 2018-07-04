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

#ifndef __LAVAENGINE_STATE_MACHINE__
#define __LAVAENGINE_STATE_MACHINE__

#include <lavaEngine/api.h>
#include <memory>
#include <string>

#include "Component.h"

namespace lava
{
  namespace engine
  {
    class StateMachine;
    typedef std::shared_ptr<StateMachine> StateMachinePtr;
    class State
    {
    public:
      State( StateMachinePtr owner_ );
      virtual ~State( void );
      virtual void enter( void ) = 0;
      virtual void exit( void ) = 0;
      virtual void update( void ) = 0;
      virtual std::string description( void ) = 0;
      StateMachinePtr owner;
    };
    typedef std::shared_ptr<State> StatePtr;
    class StateMachine: public Component
    {
      IMPLEMENT_COMPONENT( StateMachine )
    public:
      LAVAENGINE_API
      StateMachine( void );
      LAVAENGINE_API
      ~StateMachine( void );

      StatePtr currentState;

      LAVAENGINE_API
      virtual void update( const float& );
      LAVAENGINE_API
      void SwitchState( StatePtr newState );
    };
  }
}

#endif /* __LAVAENGINE_STATE_MACHINE__ */
