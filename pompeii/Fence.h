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

#ifndef __POMPEII_FENCE__
#define __POMPEII_FENCE__

#include "includes.hpp"

#include "VulkanResource.h"

#include "noncopyable.hpp"

#include <pompeii/api.h>

namespace pompeii
{
  class Fence
    : public VulkanResource
    , private NonCopyable< Fence >
  {
  public:
    POMPEII_API
    Fence( const std::shared_ptr<Device>& device, bool signaled );
    POMPEII_API
    virtual ~Fence( void );
    POMPEII_API
    bool isSignaled( void ) const;
    POMPEII_API
    void reset( void );
    POMPEII_API
    void wait( uint64_t timeout = UINT64_MAX ) const;
    POMPEII_API
    inline operator vk::Fence( void ) const
    {
      return _fence;
    }
    POMPEII_API
    static void waitForFences(
      vk::ArrayProxy<const std::shared_ptr< Fence > > fences,
      bool all, uint32_t timeout
    );
    POMPEII_API
    static void resetFences(
      vk::ArrayProxy< const std::shared_ptr< Fence > > fences );

  private:
    vk::Fence _fence;
  };
}

#endif /* __POMPEII_FENCE__ */