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

#include "StateMachineComponent.h"
#include <iostream>

namespace lava
{
  namespace engine
  {
    State::State( StateMachinePtr owner_ )
    {
      this->owner = owner_;
    }
    State::~State( void )
    {
    }

    StateMachine::StateMachine( void )
    {
      currentState = nullptr;
    }

    StateMachine::~StateMachine( void )
    {
    }

    void StateMachine::SwitchState( StatePtr newState )
    {
      if ( currentState != nullptr )
      {
        currentState->exit( );
      }

      currentState = newState;
      if ( newState != nullptr )
      {
        currentState->enter( );
      }
    }

    void StateMachine::update( const float& )
    {
      if ( currentState != nullptr )
      {
        std::cout << "State: " << currentState->description( ) << std::endl;
        currentState->update( );
      }
      //Component::Update( dt );
    }
  }
}