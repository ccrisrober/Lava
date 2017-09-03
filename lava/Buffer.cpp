#include "Buffer.h"

#include "Device.h"
#include "PhysicalDevice.h"

#include "utils.hpp"

namespace lava
{
  Buffer::Buffer( const DeviceRef& device, vk::BufferCreateFlags createFlags,
    vk::DeviceSize size, vk::BufferUsageFlags usageFlags, 
    vk::SharingMode sharingMode, vk::ArrayProxy<const uint32_t>, 
    vk::MemoryPropertyFlags memoryPropertyFlags )
    : VulkanResource( device )
  {
    vk::BufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.size = size;
    vertexBufferInfo.usage = usageFlags;
    vertexBufferInfo.sharingMode = sharingMode;

    _buffer = static_cast< vk::Device >( *_device ).createBuffer( vertexBufferInfo );
    vk::MemoryRequirements memReqs = 
      static_cast< vk::Device >( *_device ).getBufferMemoryRequirements( _buffer );

    vk::MemoryAllocateInfo memAlloc;
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = findMemoryType( 
      _device->_physicalDevice->getMemoryProperties( ), 
      memReqs.memoryTypeBits, 
      memoryPropertyFlags );

    _memory = static_cast< vk::Device >( *_device ).allocateMemory( memAlloc );
    static_cast< vk::Device >( *_device ).bindBufferMemory( _buffer, _memory, 0 );
  }

  Buffer::~Buffer( )
  {
    static_cast< vk::Device >( *_device ).destroyBuffer( _buffer );
  }
  void * Buffer::map( vk::DeviceSize offset, vk::DeviceSize length ) const
  {
    void* data;
    vk::Result result = static_cast< vk::Device >( *_device ).mapMemory( _memory, offset, length, {}, &data );
    lava::utils::translateVulkanResult( result );
    assert( result == vk::Result::eSuccess );
    return data;
  }
  void Buffer::unmap( void )
  {
    static_cast< vk::Device >( *_device ).unmapMemory( _memory );
  }
}