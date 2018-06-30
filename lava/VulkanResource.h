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

#ifndef __LAVA_VULKAN_RESOURCE__
#define __LAVA_VULKAN_RESOURCE__

#include "includes.hpp"
#include <memory>

#include <lava/Device.h>

#include <lava/api.h>

namespace lava
{
  class VulkanResource
  {
  public:
    LAVA_API
    const std::shared_ptr<Device> getDevice( void )
    {
      return _device;
    }
  protected:
    LAVA_API
    VulkanResource( const std::shared_ptr<Device>& device );

    std::shared_ptr<Device> _device;
  };
}

#endif /* __LAVA_VULKAN_RESOURCE__ */