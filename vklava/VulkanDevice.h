#pragma once
#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include "VulkanQueue.h"

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
  const VkPhysicalDeviceProperties& getDeviceProperties( ) const
  {
    return _deviceProperties;
  }

  // Returns a set of features that the application can use to check if a 
  //    specific feature is supported.
  const VkPhysicalDeviceFeatures& getDeviceFeatures( ) const
  {
    return _deviceFeatures;
  }

  // Blocks the calling thread until all operations on the device finish.
  void waitIdle( void ) const;

  /*// Returns a set of properties describing the memory of the physical device.
  const VkPhysicalDeviceMemoryProperties& getMemoryProperties( ) const
  {
    return _memoryProperties;
  }*/


  /**
  * Returns index of the queue family for the specified queue type. Returns -1 if no queues for the specified type
  * exist. There will always be a queue family for the graphics type.
  */
  uint32_t getQueueFamily( GpuQueueType type ) const
  {
    return _queueInfos[ ( int ) type ].familyIdx;
  }

private:
  friend class VulkanRenderAPI;

  // Marks the device as a primary device.
  void setIsPrimary( void )
  {
    _isPrimary = true;
  }

  VkPhysicalDevice _physicalDevice;
  VkDevice _logicalDevice;
  bool _isPrimary;
  uint32_t _deviceIdx;

  VkPhysicalDeviceProperties _deviceProperties;
  VkPhysicalDeviceFeatures _deviceFeatures;
  //VkPhysicalDeviceMemoryProperties _memoryProperties;

public:
  // Contains data about a set of queues of a specific type.
  struct QueueInfo
  {
    uint32_t familyIdx;
    std::vector<VulkanQueue*> queues;
  };
  QueueInfo _queueInfos[ GPUT_COUNT ];
};
typedef std::shared_ptr<VulkanDevice> VulkanDevicePtr;
