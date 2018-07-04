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

#ifndef __LAVA_EVENT__
#define __LAVA_EVENT__

#include "includes.hpp"

#include "VulkanResource.h"

#include <lava/api.h>

namespace lava
{
  class Device;

  class Event : public VulkanResource, 
    public std::enable_shared_from_this< Event >
  {
  public:
    LAVA_API
    Event( const std::shared_ptr<Device>& device );

    Event( const Event& rhs ) = delete;
    Event& operator=( const Event& rhs ) = delete;
    
    LAVA_API
    virtual ~Event( void );

    LAVA_API
    vk::Result getStatus( void );

    LAVA_API
    bool isSignaled( void ) const;
    LAVA_API
    void reset( void );
    LAVA_API
    void set( void );

    LAVA_API
    inline operator vk::Event( void ) const
    {
      return _event;
    }

  private:
    vk::Event _event;
  };
}

#endif /* __LAVA_EVENT__ */