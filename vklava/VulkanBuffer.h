#ifndef __VKLAVA_VULKANBUFFER__
#define __VKLAVA_VULKANBUFFER__

#include "VulkanResource.h"

namespace lava
{
  class VulkanCmdBuffer;
  class VulkanBuffer: public VulkanResource
  {
  public:
    VulkanBuffer( VulkanDevicePtr device, VkBuffer buffer, VkDeviceMemory memory );
    ~VulkanBuffer( void );
    /** Returns the internal handle to the Vulkan object. */
    VkBuffer getHandle( void ) const { return _buffer; }
    void* map( VkDeviceSize offset, VkDeviceSize length ) const;
    void copy( VulkanCmdBuffer* cb, VulkanBuffer* dst, VkDeviceSize srcOffset,
      VkDeviceSize dstOffset, VkDeviceSize length );
    void unmap( void );
  public:
    VkBuffer _buffer;
    VkDeviceMemory _memory;
  };
  
  class VulkanHardwareBuffer
  {
  public:

  };
  
  class VulkanIndexBuffer
  {
  public:
    enum IndexType
    {
      IT_16BIT, // 16-bit indices.
      IT_32BIT  // 32-bit indices.
    };
    VkIndexType getType( void ) const
    {
      switch ( op )
      {
      case IT_16BIT:
        return VK_INDEX_TYPE_UINT16;
      case IT_32BIT:
        return VK_INDEX_TYPE_UINT32;
      }

      // Unsupported type
      return VK_INDEX_TYPE_UINT32;
    }
    VulkanBuffer* getResource( uint32_t deviceIdx ) const
    {
      return nullptr;
      //return mBuffer->getResource( deviceIdx );
    }
    IndexType op;
  };

}

#endif /* __VKLAVA_VULKANBUFFER__ */