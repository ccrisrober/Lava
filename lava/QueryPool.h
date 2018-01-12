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