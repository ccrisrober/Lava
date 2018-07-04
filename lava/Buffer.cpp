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

#include "Buffer.h"
#include "PhysicalDevice.h"

#include "Log.h"

#include "Image.h"

namespace lava
{
  Buffer::Buffer( const std::shared_ptr<Device>& device, 
    vk::BufferCreateFlags createFlags, vk::DeviceSize size, 
    vk::BufferUsageFlags usageFlags, vk::SharingMode sharingMode, 
    vk::ArrayProxy<const uint32_t>, vk::MemoryPropertyFlags memoryPropFlags )
    : VulkanResource( device )
    , _memoryPropertyFlags( memoryPropFlags )
    , _size( size )
  {
    vk::BufferCreateInfo bci;
    bci.flags = createFlags;
    bci.size = _size;
    bci.usage = usageFlags;
    bci.sharingMode = sharingMode;

    _buffer = static_cast< vk::Device >( *_device ).createBuffer( bci );
    _memory = _device->allocateBufferMemory( _buffer, _memoryPropertyFlags );

    // updateDescriptor( );
  }

  Buffer::~Buffer( void )
  {
    Log::info( "Free Buffer memory" );
    _device->freeMemory( _memory );
    Log::info( "Destroy Buffer" );
    static_cast< vk::Device >( *_device ).destroyBuffer( _buffer );
  }

  void Buffer::map( vk::DeviceSize offset, vk::DeviceSize length, void * data )
  {
    vk::Result result = static_cast< vk::Device >( *_device ).mapMemory(
      _memory, offset, length, { }, &data
    );
    // lava::utils::translateVulkanResult( result );
    assert( result == vk::Result::eSuccess );
  }

  void * Buffer::map( vk::DeviceSize offset, vk::DeviceSize length ) const
  {
    void* data;
    vk::Result result = static_cast< vk::Device >( *_device ).mapMemory(
      _memory, offset, length, { }, &data
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

  vk::Result Buffer::flush( vk::DeviceSize offset, vk::DeviceSize size )
  {
    vk::MappedMemoryRange mappedRange;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    //static_cast< vk::Device >( *_device ).flushMappedMemoryRanges( { mappedRange } );
    return static_cast< vk::Device >( *_device ).flushMappedMemoryRanges( 1, &mappedRange );
  }

  void Buffer::invalidate( vk::DeviceSize size, vk::DeviceSize offset )
  {
    vk::MappedMemoryRange mappedRange;
    mappedRange.memory = _memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    static_cast< vk::Device >( *_device )
      .invalidateMappedMemoryRanges( { mappedRange } );
  }

  void Buffer::readData( vk::DeviceSize offset, vk::DeviceSize length, void* dst )
  {
    void* data = map( offset, length );
    memcpy( dst, data, length );
    unmap( );
  }
  void Buffer::read( void * dst )
  {
    readData( 0, _size, dst );
  }
  void Buffer::writeData( vk::DeviceSize offset, vk::DeviceSize length,
    const void * src )
  {
    // We can't access to VRAM, but we can copy our data to DRAM
    if ( _memoryPropertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal ) throw;
    void* data = map( offset, length );
    memcpy( data, src, length );
    //if ( flush( VK_WHOLE_SIZE, 0 ) != vk::Result::eSuccess ) throw;
    unmap( );
  }
  void Buffer::set( const void * dst )
  {
    writeData( 0, _size, dst );
  }

  void Buffer::updateDescriptor( void )
  {
    /*descriptor.buffer = shared_from_this( );
    descriptor.offset = 0;
    descriptor.range = _size;*/
  }

  /*void transfer( vk::DeviceSize offset, vk::DeviceSize size, const void* data, 
    std::shared_ptr< CommandBuffer > cmdBuffer )
  {
    if( size == VK_WHOLE_SIZE ) size = this->size - offset;
  }
  void transfer( std::shared_ptr< Buffer > src, vk::DeviceSize srcOffset, 
    vk::DeviceSize dstOffset, vk::DeviceSize size, 
    std::shared_ptr< CommandBuffer > cmdBuffer )
  {

  }
  void transfer( std::shared_ptr< Image > src,
    std::shared_ptr< CommandBuffer > cmdBuffer )
  {

  }*/

  BufferView::BufferView( const std::shared_ptr<lava::Buffer>& buffer,
    vk::Format format, vk::DeviceSize offset, vk::DeviceSize range )
    : _buffer( buffer )
  {
    if ( range == uint32_t( -1 ) )
    {
      range = buffer->getSize( ) - offset;
    }
    assert( offset + range <= buffer->getSize( ) );

    vk::BufferViewCreateInfo bvci(
      vk::BufferViewCreateFlags( ), *buffer, format, offset, range
    );
    vk::Device dev = static_cast< vk::Device >( *_buffer->getDevice( ) );
    _bufferView = dev.createBufferView( bvci );
  }

  BufferView::~BufferView( void )
  {
    static_cast< vk::Device >( *_buffer->getDevice( ) )
      .destroyBufferView( _bufferView );
  }
  VertexBuffer::VertexBuffer( const std::shared_ptr<Device>& device, 
    vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size,
      vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer//BufferType::VERTEX
      , vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible
      | vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
  void VertexBuffer::bind( std::shared_ptr<CommandBuffer>& cmd,
    unsigned int index )
  {
    cmd->bindVertexBuffer( index, shared_from_this( ), 0 );
  }
  IndexBuffer::IndexBuffer( const std::shared_ptr<Device>& device, 
    const vk::IndexType type,
    uint32_t numIndices )
    : Buffer( device, vk::BufferCreateFlags( ), calcIndexSize( type, numIndices ),
      vk::BufferUsageFlagBits::eTransferDst
      | vk::BufferUsageFlagBits::eIndexBuffer//BufferType::INDEX
      , vk::SharingMode::eExclusive, nullptr,
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
  void IndexBuffer::bind( std::shared_ptr<CommandBuffer>& cmd,
    unsigned int index )
  {
    cmd->bindIndexBuffer( shared_from_this( ), index, _type );
  }
  UniformBuffer::UniformBuffer( const std::shared_ptr<Device>& device, 
    vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size,
      vk::BufferUsageFlagBits::eUniformBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
  StorageBuffer::StorageBuffer( const std::shared_ptr<Device>& device, 
    vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size,
      vk::BufferUsageFlagBits::eStorageBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
  StorageTexelBuffer::StorageTexelBuffer( const std::shared_ptr<Device>& device,
    vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size,
      vk::BufferUsageFlagBits::eStorageTexelBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
  UniformTexelBuffer::UniformTexelBuffer( const std::shared_ptr<Device>& device,
    vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size,
      vk::BufferUsageFlagBits::eUniformTexelBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
  IndirectBuffer::IndirectBuffer( const std::shared_ptr<Device>& device,
    uint32_t drawCmdCount )
    : Buffer( device, vk::BufferCreateFlags( ), sizeof(vk::DrawIndirectCommand) * drawCmdCount,
      vk::BufferUsageFlagBits::eIndirectBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
  void IndirectBuffer::writeDrawCommand(uint32_t vertexCount,
	  uint32_t firstVertex /* 0 */,
	  uint32_t cmdIndex /* 0 */) noexcept
  {
	  const vk::DeviceSize offset = cmdIndex * sizeof(vk::DrawIndirectCommand);
	  if (void *buffer = this->map(offset, sizeof(vk::DrawIndirectCommand)))
	  {
		  vk::DrawIndirectCommand *drawCmd = reinterpret_cast<vk::DrawIndirectCommand *>(buffer);
		  drawCmd->vertexCount = vertexCount;
		  drawCmd->instanceCount = 1;
		  drawCmd->firstVertex = firstVertex;
		  drawCmd->firstInstance = 0;
		  this->unmap();
	  }
  }

  void IndirectBuffer::writeDrawCommand(uint32_t vertexCount, uint32_t instanceCount, 
	  uint32_t firstVertex, uint32_t firstInstance, uint32_t cmdIndex /* 0 */) noexcept
  {
	  const vk::DeviceSize offset = cmdIndex * sizeof(vk::DrawIndirectCommand);
	  if (void *buffer = this->map(offset, sizeof(vk::DrawIndirectCommand)))
	  {
		  vk::DrawIndirectCommand *drawCmd = reinterpret_cast<vk::DrawIndirectCommand *>(buffer);
		  drawCmd->vertexCount = vertexCount;
		  drawCmd->instanceCount = instanceCount;
		  drawCmd->firstVertex = firstVertex;
		  drawCmd->firstInstance = firstInstance;
		  this->unmap();
	  }
  }

  void IndirectBuffer::writeDrawCommand(const vk::DrawIndirectCommand& drawCmd,
	  uint32_t cmdIndex /* 0 */) noexcept
  {
	  const vk::DeviceSize offset = cmdIndex * sizeof(vk::DrawIndirectCommand);
	  if (void *buffer = this->map(offset, sizeof(vk::DrawIndirectCommand)))
	  {
		  memcpy(buffer, &drawCmd, sizeof(vk::DrawIndirectCommand));
		  this->unmap();
	  }
  }

  UniformDynamicBuffer::UniformDynamicBuffer( const std::shared_ptr<Device>& device,
    vk::DeviceSize size, uint32_t count )
    : Buffer( device, vk::BufferCreateFlags( ), size * count,
      vk::BufferUsageFlagBits::eUniformBuffer,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
    , _count( count )
  {
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = _device->getPhysicalDevice( )->
      getDeviceProperties( ).limits.minUniformBufferOffsetAlignment;
    dynamicAlignment = size;

    if ( minUboAlignment > 0 )
    {
      dynamicAlignment = ( dynamicAlignment + minUboAlignment - 1 ) & 
        ~( minUboAlignment - 1 );

      std::cout << "minUniformBufferOffsetAlignment = " << minUboAlignment << std::endl;
      std::cout << "dynamicAlignment = " << dynamicAlignment << std::endl;
      // uboDataDynamic.model = (glm::mat4*)alignedAlloc(bufferSize, dynamicAlignment);
    }
  }
  DstTransferBuffer::DstTransferBuffer( const std::shared_ptr<Device>& device,
    vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size,
      vk::BufferUsageFlagBits::eTransferDst,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
  SrcTransferBuffer::SrcTransferBuffer( const std::shared_ptr<Device>& device,
    vk::DeviceSize size )
    : Buffer( device, vk::BufferCreateFlags( ), size,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::SharingMode::eExclusive, nullptr,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent )
  {
  }
}