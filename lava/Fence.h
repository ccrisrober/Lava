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

#ifndef __LAVA_FENCE__
#define __LAVA_FENCE__

#include "includes.hpp"

#include "VulkanResource.h"

#include "noncopyable.hpp"

#include <lava/api.h>

namespace lava
{
  class Fence
    : public VulkanResource
    , private NonCopyable< Fence >
  {
  public:
    LAVA_API
    Fence( const std::shared_ptr<Device>& device, bool signaled );
    LAVA_API
    virtual ~Fence( void );
    LAVA_API
    bool isSignaled( void ) const;
    LAVA_API
    void reset( void );
    LAVA_API
    void wait( uint64_t timeout = UINT64_MAX ) const;
    LAVA_API
    inline operator vk::Fence( void ) const
    {
      return _fence;
    }
    LAVA_API
    static void waitForFences(
      vk::ArrayProxy<const std::shared_ptr< Fence > > fences,
      bool all, uint32_t timeout
    );
    LAVA_API
    static void resetFences(
      vk::ArrayProxy< const std::shared_ptr< Fence > > fences );

  private:
    vk::Fence _fence;
  };
}

#endif /* __LAVA_FENCE__ */