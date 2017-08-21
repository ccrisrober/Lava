#include "VulkanBuffer.h"
#include <assert.h>
#include "VulkanCommandBuffer.h"

namespace lava
{
  VulkanBuffer::VulkanBuffer( VulkanDevicePtr device, VkBuffer buffer, VkDeviceMemory memory )
    : VulkanResource( device )
    , _buffer( buffer )
    , _memory( memory )
  {
  }
  VulkanBuffer::~VulkanBuffer( void )
  {
    VkDevice device = _device->getLogical( );

    vkDestroyBuffer( device, _buffer, nullptr );
  }
  void* VulkanBuffer::map( VkDeviceSize offset, VkDeviceSize length ) const
  {
    VkDevice device = _device->getLogical( );

    void* data;
    VkResult result = vkMapMemory( device, _memory, offset, length, 0, &data );
    assert( result == VK_SUCCESS );

    return data;
  }
  void VulkanBuffer::copy( VulkanCmdBuffer* cb, VulkanBuffer* dst, VkDeviceSize srcOffset,
    VkDeviceSize dstOffset, VkDeviceSize length )
  {
    VkBufferCopy region;
    region.size = length;
    region.srcOffset = srcOffset;
    region.dstOffset = dstOffset;

    vkCmdCopyBuffer( cb->getHandle( ), _buffer, dst->getHandle( ), 1, &region );
  }
  void VulkanBuffer::unmap( void )
  {
    VkDevice device = _device->getLogical( );

    vkUnmapMemory( device, _memory );
  }
}