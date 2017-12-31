/**
 * Copyright (c) 2017, Lava
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

#include "Semaphore.h"

#include "Device.h"

namespace lava
{
  Semaphore::Semaphore( const DeviceRef& device )
    : VulkanResource( device )
  {
    _semaphore = vk::Device( *_device ).createSemaphore( { } );
  }
  Semaphore::~Semaphore( void )
  {
    destroy( );
  }
  void Semaphore::destroy( void )
  {
    if ( _semaphore )
    {
      vk::Device( *_device ).destroySemaphore( _semaphore );
      //_semaphore = VK_NULL_HANDLE;
    }
  }
}