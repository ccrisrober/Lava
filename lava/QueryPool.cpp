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

#include "QueryPool.h"
#include "Device.h"

namespace lava
{
  QueryPool::QueryPool( const std::shared_ptr<Device>& device, 
    vk::QueryPoolCreateFlags flags, vk::QueryType queryType, 
    uint32_t queryCount, vk::QueryPipelineStatisticFlags pipelineStatistics )
      : VulkanResource( device )
      , _queryCount( queryCount )
      , _queryType( queryType )
  {
    vk::QueryPoolCreateInfo qpci;
    qpci.flags = flags;
    qpci.queryType = queryType;
    qpci.queryCount = _queryCount;
    qpci.pipelineStatistics = pipelineStatistics;

    _query = static_cast< vk::Device >( *_device ).createQueryPool( qpci );
  }

  QueryPool::~QueryPool( void )
  {
    static_cast<vk::Device>( *_device ).destroyQueryPool( _query );
  }

  /*std::vector< uint8_t > QueryPool::getResults(uint32_t startQuery, 
    uint32_t queryCount, size_t dataSize, vk::DeviceSize stride, 
    vk::QueryResultFlags flags)
  {
    std::vector<uint8_t> data( dataSize );
    static_cast<vk::Device>( *_device )
      .getQueryPoolResults< uint8_t >( _query, startQuery, queryCount, data, 
        stride, flags );
    return data;
  }*/
  OcclusionQuery::OcclusionQuery(std::shared_ptr<Device> device, 
	uint32_t queryCount)
	: QueryPool(std::move(device), {}, 
	  vk::QueryType::eOcclusion, queryCount, { })
  {
  }
  PipelineStatisticsQuery::PipelineStatisticsQuery(
	  std::shared_ptr<Device> device, 
	  vk::QueryPipelineStatisticFlags pipelineStatistics_)
	  : QueryPool(std::move(device), {},
		  vk::QueryType::ePipelineStatistics, 1, pipelineStatistics_)
  {
  }
  TimestampQuery::TimestampQuery(std::shared_ptr<Device> device, 
	  uint32_t queryCount)
	  : QueryPool(std::move(device), {},
		  vk::QueryType::eTimestamp, queryCount, {})
  {
  }
}