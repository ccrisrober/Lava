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

#include "Event.h"
#include "Device.h"

namespace lava
{
  Event::Event( const std::shared_ptr< Device >& device )
    : VulkanResource( device )
  {
    _event = static_cast< vk::Device >( *_device ).createEvent( { } );
  }

  Event::~Event( void )
  {
    static_cast< vk::Device >( *_device ).destroyEvent( _event );
  }

  vk::Result Event::getStatus( void )
  {
    return static_cast< vk::Device >( *_device ).getEventStatus( _event );
  }

  bool Event::isSignaled( void ) const
  {
    vk::Result result = static_cast< vk::Device >( *_device )
      .getEventStatus( _event );
      
    assert( ( result == vk::Result::eEventSet ) || 
      ( result == vk::Result::eEventReset ) );
    
    return( result == vk::Result::eEventSet );
  }

  void Event::reset( void )
  {
    static_cast< vk::Device >( *_device ).resetEvent( _event );
  }

  void Event::set( void )
  {
    static_cast< vk::Device >( *_device ).setEvent( _event );
  }
}