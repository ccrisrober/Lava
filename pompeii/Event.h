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

#ifndef __POMPEII_EVENT__
#define __POMPEII_EVENT__

#include "includes.hpp"

#include "VulkanResource.h"

#include <pompeii/api.h>

namespace pompeii
{
  class Device;

  class Event : public VulkanResource, 
    public std::enable_shared_from_this< Event >
  {
  public:
    POMPEII_API
    Event( const std::shared_ptr<Device>& device );

    Event( const Event& rhs ) = delete;
    Event& operator=( const Event& rhs ) = delete;
    
    POMPEII_API
    virtual ~Event( void );

    POMPEII_API
    vk::Result getStatus( void );

    POMPEII_API
    bool isSignaled( void ) const;
    POMPEII_API
    void reset( void );
    POMPEII_API
    void set( void );

    POMPEII_API
    inline operator vk::Event( void ) const
    {
      return _event;
    }

  private:
    vk::Event _event;
  };
}

#endif /* __POMPEII_EVENT__ */