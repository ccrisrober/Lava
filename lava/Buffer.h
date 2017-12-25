/**
 * Copyright (c) 2017, Lava
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

    Buffer( const Buffer& ) = delete;
    Buffer( Buffer&& ) = delete;

    Buffer& operator=( const Buffer& ) = delete;
    Buffer& operator=( Buffer&& ) = delete;

    static vk::BufferUsageFlags getBufferUsage( const BufferType& type );

    LAVA_API
    void map( vk::DeviceSize offset, vk::DeviceSize length, void* data );
    LAVA_API
    void* map( vk::DeviceSize offset, vk::DeviceSize length ) const;
    LAVA_API
    void unmap( void );
    
    //template <typename T> void update( vk::DeviceSize offset, 
    //  vk::ArrayProxy<const T> data, std::shared_ptr<CommandBuffer> const& cmdBuff );

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
    void flush( vk::DeviceSize size, vk::DeviceSize offset = 0 );
    LAVA_API
    void invalidate( vk::DeviceSize size, vk::DeviceSize offset = 0 );

    LAVA_API
    void readData( vk::DeviceSize offset, vk::DeviceSize length, void* dst );
    LAVA_API
    void writeData( vk::DeviceSize offset, vk::DeviceSize length, const void* src );

    // todo: virtual void bind( std::shared_ptr<CommandBuffer>& cmd, unsigned int index = 0 ) = 0;
  //protected:
    vk::Buffer _buffer;
    vk::BufferView _view;
    vk::DeviceMemory _memory;

    vk::DeviceSize _size;
  };
  /*template<typename T>
  inline void Buffer::update( vk::DeviceSize offset, vk::ArrayProxy<const T> data, 
    std::shared_ptr<CommandBuffer> const & cmdBuff )
  {
    // todo
  }*/

  class VertexBuffer: public Buffer
  {
  public:
    LAVA_API
    VertexBuffer( const DeviceRef& device, vk::DeviceSize size );
    LAVA_API
    void bind( std::shared_ptr<CommandBuffer>& cmd, unsigned int index = 0 );
  };

  class IndexBuffer: public Buffer
  {
  public:
    LAVA_API
    IndexBuffer( const DeviceRef& device, const vk::IndexType type, 
      uint32_t numIndices );
    LAVA_API
    void bind( std::shared_ptr<CommandBuffer>& cmd, unsigned int index = 0 );
    inline const vk::IndexType getIndexType( void )
    {
      return _type;
    }
  protected:
    static vk::DeviceSize calcIndexSize( const vk::IndexType& type, 
      uint32_t numIndices );
    vk::IndexType _type;
  };

  class UniformBuffer : public Buffer
  {
  public:
    LAVA_API
    UniformBuffer( const DeviceRef&, vk::DeviceSize size );
  };

  class StorageBuffer : public Buffer
  {
  public:
    LAVA_API
    StorageBuffer( const DeviceRef&, vk::DeviceSize size );
  };

  class UniformTexelBuffer : public Buffer
  {
  public:
    LAVA_API
    UniformTexelBuffer( const DeviceRef&, vk::DeviceSize size );
  };

  class BufferView
  {
    public:
    LAVA_API
    BufferView(const std::shared_ptr<lava::Buffer>& buffer, 
      vk::Format format, vk::DeviceSize offset, 
      vk::DeviceSize range );
    LAVA_API
    virtual ~BufferView( void );

    LAVA_API
    inline operator vk::BufferView( void ) const
    {
      return _bufferView;
    }

    BufferView(BufferView const& rhs) = delete;
    BufferView & operator=(BufferView const& rhs) = delete;

  private:
    vk::BufferView  _bufferView;
    std::shared_ptr<lava::Buffer> _buffer;
  };
}

#endif /* __LAVA_BUFFER__ */