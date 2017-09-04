#ifndef __LAVA_BUFFER__
#define __LAVA_BUFFER__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

#include "CommandBuffer.h"

namespace lava
{
  enum class BufferType
  {
    VERTEX,
    INDEX,
    UNIFORM,
    GENERIC,
    STORAGE,
    STRUCTURED
  };
  class Buffer : public VulkanResource, private NonCopyable<Buffer>,
    public std::enable_shared_from_this<Buffer>
  {
  public:
    LAVA_API
    Buffer( const DeviceRef& device, vk::BufferCreateFlags createFlags, 
      vk::DeviceSize size, vk::BufferUsageFlags usageFlags, 
      vk::SharingMode sharingMode,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices, 
      vk::MemoryPropertyFlags memoryPropertyFlags );
    LAVA_API
    Buffer( const DeviceRef& device, vk::BufferCreateFlags createFlags, 
      vk::DeviceSize size, const BufferType& bufferType, 
      vk::SharingMode sharingMode,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices, 
      vk::MemoryPropertyFlags memoryPropertyFlags );
    LAVA_API
    virtual ~Buffer( void );

    static vk::BufferUsageFlags getBufferUsage( const BufferType& type );

    LAVA_API
    void* map( vk::DeviceSize offset, vk::DeviceSize length ) const;
    LAVA_API
    void unmap( void );
    template <typename T> void update( vk::DeviceSize offset, 
      vk::ArrayProxy<const T> data, std::shared_ptr<CommandBuffer> const& cmdBuff );

    inline operator vk::Buffer( void ) const
    {
      return _buffer;
    }

    LAVA_API
    void copy( const std::shared_ptr<CommandBuffer>& cmd, 
      std::shared_ptr<Buffer> dst, vk::DeviceSize srcOffset, 
      vk::DeviceSize dstOffset, vk::DeviceSize length );
    LAVA_API
    void copy( const std::shared_ptr<CommandBuffer>& cmd, 
      std::shared_ptr< Image > dst, const vk::Extent3D& extent, 
      const vk::ImageSubresourceLayers& range, vk::ImageLayout layout );

    LAVA_API
    void readData( vk::DeviceSize offset, vk::DeviceSize length, void* dst );
    LAVA_API
    void writeData( vk::DeviceSize offset, vk::DeviceSize length, const void* src );
  protected:
    vk::Buffer _buffer;
    vk::BufferView _view;
    vk::DeviceMemory _memory;
  };
  template<typename T>
  inline void Buffer::update( vk::DeviceSize offset, vk::ArrayProxy<const T> data, 
    std::shared_ptr<CommandBuffer> const & cmdBuff )
  {
    // todo
  }

  class VertexBuffer: public Buffer
  {
  public:
    LAVA_API
    VertexBuffer( const DeviceRef& device, vk::DeviceSize size );
  };

  class IndexBuffer: public Buffer
  {
  public:
    LAVA_API
    IndexBuffer( const DeviceRef& device, const vk::IndexType type, uint32_t numIndices );
    inline const vk::IndexType getIndexType( void )
    {
      return _type;
    }
  protected:
    static vk::DeviceSize calcIndexSize( const vk::IndexType& type, uint32_t numIndices );
    vk::IndexType _type;
  };
}

#endif /* __LAVA_BUFFER__ */