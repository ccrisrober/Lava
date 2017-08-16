#ifndef __VKLAVA_VULKANBUFFER__
#define __VKLAVA_VULKANBUFFER__

#include "VulkanResource.h"

namespace vklava
{
  /** These values represent a hint to the driver when writing to a GPU buffer. */
  enum BufferWriteType
  {
    /**
    * Default flag with least restrictions. Can cause a CPU-GPU sync point so avoid using it often (every frame)
    * as that might limit your performance significantly.
    */
    BWT_NORMAL,
    /**
    * Tells the driver to completely discard the contents of the buffer you are writing to. The driver will (most
    * likely) internally allocate another buffer with same specifications (which is fairly fast) and you will avoid
    * CPU-GPU stalls.
    */
    BWT_DISCARD,
    /**
    * Guarantees the driver that you will not be updating any part of the buffer that is currently used. This will
    * also avoid CPU-GPU stalls, without requiring you to discard the entire buffer. However it is hard to guarantee
    * when GPU has finished using a buffer.
    */
    BTW_NO_OVERWRITE
  };

  class VulkanBuffer: public VulkanResource
  {
  public:
    VulkanBuffer( VulkanDevicePtr device, VkDeviceSize size, 
      VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
      VkBuffer& buffer, VkDeviceMemory& bufferMemory );
    ~VulkanBuffer( );
  protected:
    VkBufferCreateInfo _bufferInfo;
  };
  enum IndexType
  {
    IT_16BIT,	/**< 16-bit indices. */
    IT_32BIT		/**< 32-bit indices. */
  };
  enum GpuBufferUsage
  {
    /**
    * Signifies that you don't plan on modifying the buffer often (or at all) after creation. Modifying such buffer
    * will involve a larger performance hit. Mutually exclusive with GBU_DYNAMIC.
    */
    GBU_STATIC,
    /**
    * Signifies that you will modify this buffer fairly often (e.g. every frame). Mutually exclusive with GBU_STATIC.
    */
    GBU_DYNAMIC,
  };

  struct INDEX_BUFFER_DESC
  {
    IndexType indexType; /**< Index type, determines the size of a single index. */
    uint32_t numIndices; /**< Number of indices can buffer can hold. */
    GpuBufferUsage usage = GBU_STATIC; /**< Usage that tells the hardware how will be buffer be used. */
  };

  class VulkanIndexBuffer
  {
  public:
    VulkanIndexBuffer( const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask )
      : IndexBuffer( desc, deviceMask )
      , _buffer( nullptr )
      , _usage( desc.usage )
      , _deviceMask( deviceMask )
    {
    }
    ~VulkanIndexBuffer( void )
    {
      if ( _buffer != nullptr )
      {
        delete _buffer;
      }
    }

    void readData( uint32_t offset, uint32_t length, void* dest,
      uint32_t deviceIdx = 0, uint32_t queueIdx = 0 )
    {
      _buffer->readData( offset, length, dest, deviceIdx, queueIdx );
    }
    void writeData( uint32_t offset, uint32_t length, const void* source,
      BufferWriteType writeFlags = BWT_NORMAL, uint32_t queueIdx = 0 );// override;
    {
      _buffer->writeData( offset, length, source, writeFlags, queueIdx );
    }
    /**
    * Gets the resource wrapping the buffer object, on the specified device. 
    * If GPU param block buffer's device mask
    * doesn't include the provided device, null is returned.
    */
    VulkanBuffer* getResource( uint32_t deviceIdx ) const
    {
      return _buffer->getResource( deviceIdx );
    }

  protected:
    void* map( uint32_t offset, uint32_t length, GpuLockOptions options,
      uint32_t deviceIdx, uint32_t queueIdx )// override;
    {
      return _buffer->lock( offset, length, options, deviceIdx, queueIdx );
    }

    void unmap( void )// override;
    {
      _buffer->unlock( );
    }

    void initialize( void )// override;
    {
      _buffer = new VulkanHardwareBuffer( VulkanHardwareBuffer::BT_INDEX, 
        BF_UNKNOWN, mUsage, mSize, mDeviceMask );
    }

  private:
    VulkanHardwareBuffer* _buffer;
    GpuBufferUsage _usage;
    GpuDeviceFlags _deviceMask;
  };
}

#endif /* __VKLAVA_VULKANBUFFER__ */