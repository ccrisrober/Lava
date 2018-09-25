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

#ifndef __POMPEII_SEMAPHORE__
#define __POMPEII_SEMAPHORE__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace pompeii
{
  class Semaphore
    : public VulkanResource
    , private NonCopyable< Semaphore >
  {
  public:
    POMPEII_API
    Semaphore( const std::shared_ptr<Device>& device );
    Semaphore( const Semaphore& ) = delete;

    Semaphore& operator=( const Semaphore& ) = delete;
    virtual ~Semaphore( void );

    POMPEII_API
    inline operator vk::Semaphore( void )
    {
      return _semaphore;
    }

  protected:
    vk::Semaphore _semaphore;
  };
}

#endif /* __POMPEII_SEMAPHORE__ */