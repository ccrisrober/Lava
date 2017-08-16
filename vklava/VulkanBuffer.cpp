#include "VulkanBuffer.h"
#include <assert.h>

namespace vklava
{
  VulkanBuffer::VulkanBuffer( VulkanDevicePtr device, VkDeviceSize size, 
    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
    VkBuffer& buffer, VkDeviceMemory& bufferMemory )
    : VulkanResource( device )
  {
    _bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    _bufferInfo.pNext = nullptr;
    _bufferInfo.flags = 0;
    _bufferInfo.queueFamilyIndexCount = 0;
    _bufferInfo.pQueueFamilyIndices = nullptr;
    _bufferInfo.size = size;
    _bufferInfo.usage = usage;
    _bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer( *_device, &_bufferInfo, nullptr, &buffer );
    assert( result == VK_SUCCESS );

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements( *_device, buffer, &memRequirements );

    bufferMemory = _device->allocateMemory( memRequirements, properties );

    result = vkBindBufferMemory( *_device, buffer, bufferMemory, 0 );
    assert( result == VK_SUCCESS );
  }

  VulkanBuffer::~VulkanBuffer( )
  {

  }
}