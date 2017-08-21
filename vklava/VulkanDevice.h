#ifndef __VKLAVA_VULKANDEVICE__
#define __VKLAVA_VULKANDEVICE__

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <array>

#include "VulkanQueue.h"

namespace lava
{
  class VulkanCmdBufferPool;

  class VulkanDevice
  {
  public:
    VulkanDevice( VkPhysicalDevice device, uint32_t deviceIdx );
    ~VulkanDevice( void );
    // Returns an object describing the physical properties of the device.
    VkPhysicalDevice getPhysical( void ) const
    {
      return _physicalDevice;
    }

    // Returns an object describing the logical properties of the device.
    VkDevice getLogical( void ) const
    {
      return _logicalDevice;
    }

    // Returns true if the device is one of the primary GPU's.
    bool isPrimary( void ) const
    {
      return _isPrimary;
    }

    // Returns the unique index of the device.
    uint32_t getIndex( void ) const
    {
      return _deviceIdx;
    }

    // Returns a set of properties describing the physical device.
    const VkPhysicalDeviceProperties& getDeviceProperties( void ) const
    {
      return _deviceProperties;
    }

    // Returns a set of features that the application can use to check if a 
    //    specific feature is supported.
    const VkPhysicalDeviceFeatures& getDeviceFeatures( void ) const
    {
      return _deviceFeatures;
    }

    // Returns a set of properties describing the memory of the physical device.
    const VkPhysicalDeviceMemoryProperties& getMemoryProperties( void ) const
    {
      return _memoryProperties;
    }

    // Returns the number of queue supported on the device, per type.
    uint32_t getNumQueues( GpuQueueType type ) const
    {
      return ( uint32_t ) _queueInfos[ ( int ) type ].queues.size( );
    }

    // Returns queue of the specified type at the specified index. Index must be in range [0, getNumQueues()).
    VulkanQueue* getQueue( GpuQueueType type, uint32_t idx ) const
    {
      return _queueInfos[ ( int ) type ].queues[ idx ];
    }

    VulkanQueue* getGraphicQueue( void )
    {
      return getQueue( GpuQueueType::GPUT_GRAPHICS, 0 );
    }

    VulkanQueue* getComputeQueue( void )
    {
      return getQueue( GpuQueueType::GPUT_COMPUTE, 0 );
    }

    VulkanQueue* getTransferQueue( void )
    {
      return getQueue( GpuQueueType::GPUT_TRANSFER, 0 );
    }

    /**
    * Returns index of the queue family for the specified queue type.
    * Returns -1 if no queues for the specified type exist.
    * There will always be a queue family for the graphics type.
    */
    uint32_t getQueueFamily( GpuQueueType type ) const
    {
      return _queueInfos[ ( int ) type ].familyIdx;
    }

    // Blocks the calling thread until all operations on the device finish.
    void waitIdle( void ) const;

    operator VkDevice( void )
    {
      return _logicalDevice;
    }

    /**
    * Allocates memory for the provided image, and binds it to the image.
    * Returns null if it cannot find memory with the specified flags.
    */
    VkDeviceMemory allocateImageMemory( VkImage image, VkMemoryPropertyFlags flags );

    /**
    * Allocates memory for the provided buffer, and binds it to the buffer.
    * Returns null if it cannot find memory with the specified flags.
    */
    VkDeviceMemory allocateBufferMemory( VkBuffer buffer, VkMemoryPropertyFlags flags );

    /**
    * Allocates a block of memory according to the provided memory requirements.
    * Returns null if it cannot find memory with the specified flags.
    */
    VkDeviceMemory allocateMemReqMemory( const VkMemoryRequirements& reqs,
      VkMemoryPropertyFlags flags );

    // Frees a previously allocated block of memory.
    void freeMemory( VkDeviceMemory memory );

    /** Returns a pool that can be used for allocating command buffers 
     * for all queues on this device.
     */
    VulkanCmdBufferPool& getCmdBufferPool( void ) const
    {
      return *_commandBufferPool;
    }
  private:
    friend class VulkanRenderAPI;

    // Attempts to find a memory type that matches the requirements bits and the requested flags.
    uint32_t findMemoryType( uint32_t requirementBits,
      VkMemoryPropertyFlags wantedFlags );

    // Marks the device as a primary device.
    void setIsPrimary( void )
    {
      _isPrimary = true;
    }

    VkPhysicalDevice _physicalDevice;
    VkDevice _logicalDevice;
    bool _isPrimary;
    uint32_t _deviceIdx;

    VulkanCmdBufferPool * _commandBufferPool;

    VkPhysicalDeviceProperties _deviceProperties;
    VkPhysicalDeviceFeatures _deviceFeatures;
    VkPhysicalDeviceMemoryProperties _memoryProperties;

  public:
    // Contains data about a set of queues of a specific type.
    struct QueueInfo
    {
      uint32_t familyIdx = -1;
      std::vector<VulkanQueue*> queues;
    };
    std::array< QueueInfo, GPUT_COUNT> _queueInfos;
  };
  typedef std::shared_ptr<VulkanDevice> VulkanDevicePtr;
}

#endif /* __VKLAVA_VULKANDEVICE__ */