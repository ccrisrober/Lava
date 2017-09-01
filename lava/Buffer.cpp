#include "Buffer.h"

#include "Device.h"
#include "PhysicalDevice.h"

namespace lava
{
  Buffer::Buffer( const DeviceRef& device, vk::BufferCreateFlags createFlags, 
    vk::DeviceSize size, vk::BufferUsageFlags usageFlags, 
    vk::SharingMode sharingMode, vk::ArrayProxy<const uint32_t> queueFamilyIndices, 
    vk::MemoryPropertyFlags memoryPropertyFlags )
    : VulkanResource( device )
  {
    vk::BufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.size = size;
    // Buffer is used as the copy source
    vertexBufferInfo.usage = usageFlags;
    vertexBufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;

    // Copy vertex data to a buffer visible to the host
    _buffer = static_cast< vk::Device >( *_device ).createBuffer( vertexBufferInfo );
    vk::MemoryRequirements memReqs = static_cast< vk::Device >( *_device ).getBufferMemoryRequirements( _buffer );

    vk::MemoryAllocateInfo memAlloc;
    memAlloc.allocationSize = memReqs.size;
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is host visible memory, and VK_MEMORY_PROPERTY_HOST_COHERENT_BIT makes sure writes are directly visible
    memAlloc.memoryTypeIndex = findMemoryType( 
      _device->_physicalDevice->getMemoryProperties( ), 
      memReqs.memoryTypeBits, 
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );

    _memory = static_cast< vk::Device >( *_device ).allocateMemory( memAlloc );
    static_cast< vk::Device >( *_device ).bindBufferMemory( _buffer, _memory, 0 );

    /*vk::BufferCreateInfo bci( createFlags, size, usageFlags, sharingMode, queueFamilyIndices.size( ), queueFamilyIndices.data( ) );
    _buffer = static_cast< vk::Device >( *_device ).createBuffer( bci );
    vk::MemoryRequirements memReqs = static_cast< vk::Device >( *_device ).getBufferMemoryRequirements( _buffer );
    //uint32_t memoryTypeIndex = findMemoryType( _device->_physicalDevice->getMemoryProperties( ), memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal );
    //assert( memoryTypeIndex != -1 );

    _memory = _device->allocateMemReqMemory( memReqs, vk::MemoryPropertyFlagBits::eDeviceLocal );
    vk::Device( *_device ).bindBufferMemory( _buffer, _memory, 0 );*/
  }

  Buffer::~Buffer( )
  {
    static_cast< vk::Device >( *_device ).destroyBuffer( _buffer );
  }
  void * Buffer::map( vk::DeviceSize offset, vk::DeviceSize length ) const
  {
    void* data;
    vk::Result result = static_cast< vk::Device >( *_device ).mapMemory( _memory, offset, length, {}, &data );
    assert( result == vk::Result::eSuccess );
    return data;
  }
  void Buffer::unmap( void )
  {
    static_cast< vk::Device >( *_device ).unmapMemory( _memory );
  }
}