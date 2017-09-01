#ifndef __LAVA_BUFFER__
#define __LAVA_BUFFER__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

#include "CommandBuffer.h"

namespace lava
{
  class Buffer : public VulkanResource, private NonCopyable<Buffer>
  {
  public:
    LAVA_API
    Buffer( const DeviceRef& device, vk::BufferCreateFlags createFlags, vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vk::SharingMode sharingMode,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices, vk::MemoryPropertyFlags memoryPropertyFlags );
    LAVA_API
    virtual ~Buffer( );

    LAVA_API
    void* map( vk::DeviceSize offset, vk::DeviceSize length ) const;
    LAVA_API
    void unmap( void );
    template <typename T> void update( vk::DeviceSize offset, 
      vk::ArrayProxy<const T> data, std::shared_ptr<CommandBuffer> const& cmdBuff );

    inline operator vk::Buffer( ) const
    {
      return _buffer;
    }

  protected:
    vk::Buffer _buffer;
    vk::BufferView _view;
    vk::DeviceMemory _memory;
  };
  template<typename T>
  inline void Buffer::update( vk::DeviceSize offset, vk::ArrayProxy<const T> data, 
    std::shared_ptr<CommandBuffer> const & cmdBuff )
  {
    /*size_t size = data.size( ) * sizeof( T );
    if ( ( ( offset & 0x3 ) == 0 ) && ( size < 64 * 1024 ) && ( ( size & 0x3 ) == 0 ) )
    {
      cmdBuff->updateBuffer( shared_from_this( ), offset, data );
    }
    else if ( getMemoryPropertyFlags( ) & vk::MemoryPropertyFlagBits::eHostVisible )
    {
      void * pData = get<DeviceMemory>( )->map( offset, size );
      memcpy( pData, data.data( ), size );
      if ( !( getMemoryPropertyFlags( ) & vk::MemoryPropertyFlagBits::eHostCoherent ) )
      {
        get<DeviceMemory>( )->flush( offset, size );
      }
      get<DeviceMemory>( )->unmap( );
    }
    else
    {
      std::shared_ptr<Buffer> mappingBuffer = get<Device>( )->createBuffer( m_size, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, nullptr, vk::MemoryPropertyFlagBits::eHostVisible,
        nullptr, get<Allocator>( ) );
      void * pData = mappingBuffer->get<DeviceMemory>( )->map( offset, size );
      memcpy( pData, data.data( ), size );
      mappingBuffer->get<DeviceMemory>( )->flush( offset, size );
      mappingBuffer->get<DeviceMemory>( )->unmap( );
      commandBuffer->copyBuffer( mappingBuffer, shared_from_this( ), vk::BufferCopy( 0, 0, size ) );
    }*/
  }
}

#endif /* __LAVA_BUFFER__ */