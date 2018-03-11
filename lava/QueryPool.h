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

#ifndef __LAVA_QUERYPOOL__
#define __LAVA_QUERYPOOL__

#include "includes.hpp"

#include "VulkanResource.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class QueryPool : public VulkanResource, 
    public std::enable_shared_from_this< QueryPool >
  {
  public:
    LAVA_API
    QueryPool( const std::shared_ptr<Device>& device, 
      vk::QueryPoolCreateFlags flags, vk::QueryType queryType, 
      uint32_t entryCount, vk::QueryPipelineStatisticFlags pipelineStatistics );

    QueryPool( const QueryPool& rhs ) = delete;
    QueryPool& operator=( const QueryPool& rhs ) = delete;

    LAVA_API
    virtual ~QueryPool( void );

    LAVA_API
    std::vector< uint8_t > getResults( uint32_t startQuery, uint32_t queryCount, 
      size_t dataSize, vk::DeviceSize stride, vk::QueryResultFlags flags );

    LAVA_API
    inline operator vk::QueryPool( void ) const
    {
      return _query;
    }

    LAVA_API
    inline vk::QueryType getQueryType( void ) const
    {
      return _queryType;
    }

  private:
    uint32_t _queryCount;
    vk::QueryPool _query;
    vk::QueryType _queryType;
  };
}

#endif /* __LAVA_QUERYPOOL__ */