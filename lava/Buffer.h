/**
 * Copyright (c) 2017 - 2018, Lava
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

#include <lava/CommandBuffer.h>
#include <lava/Device.h>
#include <lava/VulkanResource.h>
#include <memory>

namespace lava
{
  class Image;

  class Buffer : public VulkanResource, private NonCopyable<Buffer>,
    public std::enable_shared_from_this<Buffer>
  {
  public:
    LAVA_API
      Buffer( const std::shared_ptr<Device>& device, 
        vk::BufferCreateFlags createFlags, vk::DeviceSize size, 
        vk::BufferUsageFlags usageFlags, vk::SharingMode sharingMode,
        vk::ArrayProxy<const uint32_t> queueFamilyIndices,
        vk::MemoryPropertyFlags memoryPropertyFlags );
    /*LAVA_API
      Buffer( const std::shared_ptr<Device>& device, vk::BufferCreateFlags createFlags,
        vk::DeviceSize size, const BufferType& bufferType,
        vk::SharingMode sharingMode,
        vk::ArrayProxy<const uint32_t> queueFamilyIndices,
        vk::MemoryPropertyFlags memoryPropertyFlags );*/
    LAVA_API
    virtual ~Buffer( void );

    Buffer( const Buffer& ) = delete;
    Buffer( Buffer&& ) = delete;

    Buffer& operator=( const Buffer& ) = delete;
    Buffer& operator=( Buffer&& ) = delete;

    //static vk::BufferUsageFlags getBufferUsage( const BufferType& type );

    LAVA_API
    void map( vk::DeviceSize offset, vk::DeviceSize length, void* data );
    LAVA_API
    void* map( vk::DeviceSize offset, vk::DeviceSize length ) const;
    LAVA_API
    void unmap( void );

    LAVA_API
    inline operator vk::Buffer( void ) const { return _buffer; }

    LAVA_API
    void copy( const std::shared_ptr<CommandBuffer>& cmd,
      std::shared_ptr<Buffer> dst, vk::DeviceSize srcOffset,
      vk::DeviceSize dstOffset, vk::DeviceSize length );
    LAVA_API
    void copy( const std::shared_ptr<CommandBuffer>& cmd,
      std::shared_ptr< Image > dst, const vk::Extent3D& extent,
      const vk::ImageSubresourceLayers& range, vk::ImageLayout layout );

    LAVA_API
    vk::Result flush( vk::DeviceSize offset, vk::DeviceSize size );
    LAVA_API
    void invalidate( vk::DeviceSize size, vk::DeviceSize offset = 0 );

    LAVA_API
    void readData( vk::DeviceSize offset, vk::DeviceSize length, void* dst );
    LAVA_API
    void read( void* dst );
    LAVA_API
    void writeData( vk::DeviceSize offset, vk::DeviceSize length, const void* src );
    LAVA_API
    void set( const void* src );

    LAVA_API
    void updateDescriptor( void );

    LAVA_API
    inline vk::DeviceSize getSize( void ) const { return _size; }

    /*LAVA_API
    void transfer( vk::DeviceSize offset, vk::DeviceSize size, const void* data, 
      std::shared_ptr< CommandBuffer > cmdBuffer );
    LAVA_API
    void transfer( std::shared_ptr< Buffer > src, vk::DeviceSize srcOffset, 
      vk::DeviceSize dstOffset, vk::DeviceSize size, 
      std::shared_ptr< CommandBuffer > cmdBuffer );
    LAVA_API
    void transfer( std::shared_ptr< Image > src,
      std::shared_ptr< CommandBuffer > cmdBuffer );*/

    template <typename T>
    void update( const std::shared_ptr<CommandBuffer>& commandBuffer,
      vk::DeviceSize offset, vk::ArrayProxy<const T> data );

    LAVA_API
    inline vk::MemoryPropertyFlags getMemoryPropertyFlags( void ) const
    {
      return _memoryPropertyFlags;
    }

    //protected:
    vk::Buffer _buffer;
    vk::MemoryPropertyFlags _memoryPropertyFlags;
    vk::DeviceMemory _memory;
  protected:
    vk::DeviceSize _size;
    //DescriptorBufferInfo descriptor;
  };

  class BufferView
  {
  public:
    LAVA_API
    BufferView( const std::shared_ptr<lava::Buffer>& buffer,
      vk::Format format, vk::DeviceSize offset,
      vk::DeviceSize range = VK_WHOLE_SIZE );
    LAVA_API
    virtual ~BufferView( void );

    LAVA_API
    inline operator vk::BufferView( void ) const
    {
      return _bufferView;
    }

    BufferView( BufferView const& rhs ) = delete;
    BufferView & operator=( BufferView const& rhs ) = delete;

  private:
    vk::BufferView  _bufferView;
    std::shared_ptr<lava::Buffer> _buffer;
  };

  template <typename T>
  inline void Buffer::update( const std::shared_ptr<CommandBuffer>& cmdBuff,
    vk::DeviceSize offset, vk::ArrayProxy<const T> data )
  {
    size_t size = data.size( ) * sizeof( T );
    if ( ( ( offset & 0x3 ) == 0 ) && ( size < 64 * 1024 ) &&
      ( ( size & 0x3 ) == 0 ) )
    {
      cmdBuff->updateBuffer( shared_from_this( ), offset, data );
    }
    else if ( getMemoryPropertyFlags( ) &
      vk::MemoryPropertyFlagBits::eHostVisible )
    {
      void* pData = this->map( offset, size );
      memcpy( pData, data.data( ), size );
      if ( !( getMemoryPropertyFlags( ) &
        vk::MemoryPropertyFlagBits::eHostCoherent ) )
      {
        this->flush( size, offset );
      }
      this->unmap( );
    }
    else
    {
      std::shared_ptr<Buffer> stagingBuffer = _device->createBuffer(
        _size, vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible
      );
      void * pData = stagingBuffer->map( offset, size );
      memcpy( pData, data.data( ), size );
      stagingBuffer->flush( offset, size );
      stagingBuffer->unmap( );
      cmdBuff->copyBuffer( stagingBuffer, shared_from_this( ),
        vk::BufferCopy( 0, 0, size ) );
    }
  }


  class VertexBuffer : public Buffer
  {
  public:
    LAVA_API
    VertexBuffer( const std::shared_ptr<Device>& device, vk::DeviceSize size );
    LAVA_API
    void bind( std::shared_ptr<CommandBuffer>& cmd, unsigned int index = 0 );
  };

  class IndexBuffer : public Buffer
  {
  public:
    LAVA_API
    IndexBuffer( const std::shared_ptr<Device>& device, const vk::IndexType type,
        uint32_t numIndices );
    LAVA_API
    void bind( std::shared_ptr<CommandBuffer>& cmd, unsigned int index = 0 );
    LAVA_API
    inline vk::IndexType getIndexType( void ) const
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
    UniformBuffer( const std::shared_ptr<Device>&, vk::DeviceSize size );
  };

  class StorageBuffer : public Buffer
  {
  public:
    LAVA_API
    StorageBuffer( const std::shared_ptr<Device>&, vk::DeviceSize size );
  };

  class StorageTexelBuffer : public Buffer
  {
  public:
    LAVA_API
    StorageTexelBuffer( const std::shared_ptr<Device>&, vk::DeviceSize size );
  };

  class UniformTexelBuffer : public Buffer
  {
  public:
    LAVA_API
    UniformTexelBuffer( const std::shared_ptr<Device>&, vk::DeviceSize size );
  };

  class IndirectBuffer: public Buffer
  {
  public:
    LAVA_API
    IndirectBuffer( const std::shared_ptr<Device>& device, uint32_t drawCmdCount = 1);

	LAVA_API
	void writeDrawCommand(uint32_t vertexCount,
		uint32_t firstVertex = 0,
		uint32_t cmdIndex = 0) noexcept;
	LAVA_API
	void writeDrawCommand(uint32_t vertexCount,
		uint32_t instanceCount,
		uint32_t firstVertex,
		uint32_t firstInstance,
		uint32_t cmdIndex = 0) noexcept;
	LAVA_API
	void writeDrawCommand(const vk::DrawIndirectCommand& drawCmd,
		uint32_t cmdIndex = 0) noexcept;
	//LAVA_API
	//void writeDrawCommands(const std::vector<vk::DrawIndirectCommand>& drawCmdList) noexcept;
  };

  class UniformDynamicBuffer : public Buffer
  {
  public:
    LAVA_API
    UniformDynamicBuffer( const std::shared_ptr<Device>&, vk::DeviceSize size, 
      uint32_t count );
    uint32_t dynamicAlignment;
  protected:
    uint32_t _count;
  };

  /* A buffer that can be used as the destination of a transfer command. */
  class DstTransferBuffer: public Buffer
  {
  public:
    LAVA_API
    DstTransferBuffer( const std::shared_ptr<Device>&, vk::DeviceSize size );
  };

  /* A buffer that can be used as the source of a transfer command. */
  class SrcTransferBuffer: public Buffer
  {
  public:
    LAVA_API
    SrcTransferBuffer( const std::shared_ptr<Device>&, vk::DeviceSize size );
  };

}

#endif /* __LAVA_BUFFER__ */