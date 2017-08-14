#include "VulkanCommandBuffer.h"
#include <assert.h>

VulkanCmdBufferPool::VulkanCmdBufferPool( VulkanDevicePtr device )
  : _device( device )
  , _nextId( 1 )
{
  for ( uint32_t i = 0; i < GpuQueueType::GPUT_COUNT; ++i )
  {
    uint32_t familyIdx = _device->getQueueFamily( ( GpuQueueType ) i );

    if ( familyIdx == ( uint32_t ) -1 )
      continue;

    VkCommandPoolCreateInfo poolCI;
    poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.pNext = nullptr;
    poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCI.queueFamilyIndex = familyIdx;

    PoolInfo& poolInfo = _pools[ familyIdx ];
    poolInfo.queueFamily = familyIdx;
    poolInfo.buffers.clear( );
    //memset( poolInfo.buffers, 0, sizeof( poolInfo.buffers ) );

    VkResult result = vkCreateCommandPool( _device->getLogical( ), 
      &poolCI, nullptr, &poolInfo.pool );
    assert( result == VK_SUCCESS );
  }
}


VulkanCmdBufferPool::~VulkanCmdBufferPool( )
{
  for ( auto& entry : _pools )
  {
    PoolInfo& poolInfo = entry.second;
    for ( uint32_t i = 0; i < poolInfo.buffers.size( ); ++i )
    {
      VulkanCmdBuffer* buffer = poolInfo.buffers[ i ];
      if ( buffer == nullptr )
        break;

      delete buffer;
    }

    vkDestroyCommandPool( _device->getLogical( ), poolInfo.pool, nullptr );
  }
}

VulkanCmdBuffer* VulkanCmdBufferPool::createBuffer( uint32_t queueFamily, bool secondary )
{
  auto iterFind = _pools.find( queueFamily );
  if ( iterFind == _pools.end( ) )
    return nullptr;

  const PoolInfo& poolInfo = iterFind->second;

  return new VulkanCmdBuffer( _device, _nextId++, poolInfo.pool, 
    poolInfo.queueFamily, secondary );
}


VulkanCmdBuffer::VulkanCmdBuffer( VulkanDevicePtr device, uint32_t id, 
  VkCommandPool pool, uint32_t queueFamily, bool secondary )
{

}