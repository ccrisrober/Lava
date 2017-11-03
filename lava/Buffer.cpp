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
    , _size( size )
  {
    vk::BufferCreateInfo bci;
    bci.size = _size;
    bci.usage = usageFlags;
    bci.sharingMode = sharingMode;

    _buffer = static_cast< vk::Device >( *_device ).createBuffer( bci );
    _memory = _device->allocateBufferMemory( _buffer, memoryPropertyFlags );
  }

  Buffer::Buffer( const DeviceRef& device, vk::BufferCreateFlags createFlags, 
    vk::DeviceSize size, const BufferType& bufferType, 
    vk::SharingMode sharingMode,
    vk::ArrayProxy<const uint32_t> queueFamilyIndices, 
    vk::MemoryPropertyFlags memoryPropertyFlags )
    : Buffer( device, createFlags, size, Buffer::getBufferUsage( bufferType ), 
      sharingMode, queueFamilyIndices, memoryPropertyFlags )
  {
  }

  Buffer::~Buffer( void )
  {
    if ( _view )
    {
      std::cerr << "Destroy Buffer view" << std::endl;
      static_cast<vk::Device>( *_device ).destroyBufferView( _view );
    }
    std::cerr << "Free Buffer memory" << std::endl;
    _device->freeMemory( _memory );
    std::cerr << "Destroy Buffer" << std::endl;
    static_cast< vk::Device >( *_device ).destroyBuffer( _buffer );
  }

  vk::BufferUsageFlags Buffer::getBufferUsage( const BufferType& type )
  {
    vk::BufferUsageFlags usageFlags;
    switch( type )
    {
    case BufferType::VERTEX:
      usageFlags = vk::BufferUsageFlagBits::eTransferDst 
        | vk::BufferUsageFlagBits::eVertexBuffer;
      break;
    case BufferType::INDEX:
      usageFlags = vk::BufferUsageFlagBits::eTransferDst 
        | vk::BufferUsageFlagBits::eIndexBuffer;
      break;
    case BufferType::UNIFORM:
      usageFlags = vk::BufferUsageFlagBits::eTransferDst 
        | vk::BufferUsageFlagBits::eUniformBuffer;
      break;
    case BufferType::GENERIC:
      usageFlags = vk::BufferUsageFlagBits::eTransferDst 
        | vk::BufferUsageFlagBits::eUniformTexelBuffer;
      // todo _requiresView = true;
      break;
    case BufferType::STORAGE:
      usageFlags = vk::BufferUsageFlagBits::eTransferDst 
        | vk::BufferUsageFlagBits::eUniformTexelBuffer 
        | vk::BufferUsageFlagBits::eStorageTexelBuffer;
      // todo _requiresView = true;
      break;
    case BufferType::STRUCTURED:
      usageFlags = vk::BufferUsageFlagBits::eTransferDst 
        | vk::BufferUsageFlagBits::eStorageBuffer;
      break;
    default:
      throw;
    }
    return usageFlags;
  }

  void * Buffer::map( vk::DeviceSize offset, vk::DeviceSize length ) const
  {
    void* data;
    vk::Result result = static_cast< vk::Device >( *_device ).mapMemory( 
      _memory, offset, length, {}, &data
    );
    // lava::utils::translateVulkanResult( result );
    assert( result == vk::Result::eSuccess );
    return data;
  }
  void Buffer::unmap( void )
  {
    static_cast< vk::Device >( *_device ).unmapMemory( _memory );
  }
  void Buffer::copy( const std::shared_ptr<CommandBuffer>& cmd, 
    std::shared_ptr<Buffer> dst, vk::DeviceSize srcOffset, 
    vk::DeviceSize dstOffset, vk::DeviceSize length )
  {
    vk::BufferCopy region;
    region.size = length;
    region.srcOffset = srcOffset;
    region.dstOffset = dstOffset;

    cmd->copyBuffer( shared_from_this( ), dst, region );
  }
  void Buffer::copy( const std::shared_ptr<CommandBuffer>& cmd, 
    std::shared_ptr< Image > dst, const vk::Extent3D& extent, 
    const vk::ImageSubresourceLayers& range, vk::ImageLayout layout )
  {
    vk::BufferImageCopy region;
    region.bufferRowLength = 0; //rowPitch;
    region.bufferImageHeight = 0; //sliceHeight;
    region.bufferOffset = 0;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent = extent;
    region.imageSubresource = range;

    cmd->copyBufferToImage( shared_from_this( ), dst, layout, region );
  }


  void Buffer::flush( vk::DeviceSize size, vk::DeviceSize offset )
  {
    vk::MappedMemoryRange mappedRange;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    static_cast< vk::Device >( *_device ).flushMappedMemoryRanges( { mappedRange } );
  }

  void Buffer::readData( vk::DeviceSize offset, vk::DeviceSize length, void* dst )
  {
    void* data = map( offset, length );
    memcpy( dst, data, length );
    unmap( );
  }
  void Buffer::writeData( vk::DeviceSize offset, vk::DeviceSize length, 
    const void * src )
  {
    void* data = map( offset, length );
    memcpy( data, src, length );
    unmap( );
  }


  VertexBuffer::VertexBuffer( const DeviceRef& device, vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size, 
      BufferType::VERTEX, vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible 
        | vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
  void VertexBuffer::bind( std::shared_ptr<CommandBuffer>& cmd, unsigned int index )
  {
    cmd->bindVertexBuffer( index, shared_from_this( ), 0 );
  }
  IndexBuffer::IndexBuffer( const DeviceRef& device, const vk::IndexType type, 
    uint32_t numIndices )
    : Buffer( device, vk::BufferCreateFlags( ), calcIndexSize(type, numIndices), 
      BufferType::INDEX, vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible 
        | vk::MemoryPropertyFlagBits::eHostCoherent )
    , _type( type )
  {
  }
  vk::DeviceSize IndexBuffer::calcIndexSize( const vk::IndexType& type, 
    uint32_t numIndices )
  {
    switch ( type )
    {
    case vk::IndexType::eUint16:
      return sizeof( unsigned short ) * numIndices;
    default:
    case vk::IndexType::eUint32:
      return sizeof( unsigned int ) * numIndices;
    }
  }
  void IndexBuffer::bind( std::shared_ptr<CommandBuffer>& cmd, unsigned int index )
  {
    cmd->bindIndexBuffer( shared_from_this( ), index, _type );
  }
  UniformBuffer::UniformBuffer( const DeviceRef& device, vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size,
      vk::BufferUsageFlagBits::eUniformBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
}