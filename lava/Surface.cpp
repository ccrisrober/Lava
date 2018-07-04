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

#include "Surface.h"

#include "Instance.h"

namespace lava
{
  Surface::Surface( const std::shared_ptr< Instance >& instance, 
    const vk::SurfaceKHR& surface, bool selfDestroy )
    : _instance( instance )
    , _surface( surface )
    , _selfDestroy( selfDestroy )
  {
  }
  Surface::~Surface( void )
  {
    if ( _selfDestroy )
    {
      static_cast< vk::Instance > ( *_instance ).destroySurfaceKHR( _surface );
    }
  }
}