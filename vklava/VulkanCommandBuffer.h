#pragma once

#include "VulkanDevice.h"
#include <unordered_map>
#include <vector>

class VulkanCmdBuffer;
class VulkanCmdBufferPool
{
public:
  VulkanCmdBufferPool( VulkanDevicePtr device );
  ~VulkanCmdBufferPool( );


  /**
  * Attempts to find a free command buffer, or creates a new one if not found. Caller must guarantee the provided
  * queue family is valid.
  */
  //VulkanCmdBuffer* getBuffer( uint32_t queueFamily, bool secondary );

protected:
  //VulkanDevicePtr _device;

  // Command buffer pool and related information.
  struct PoolInfo
  {
    VkCommandPool pool = VK_NULL_HANDLE;
    std::vector<VulkanCmdBuffer*> buffers;  // [ BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY ];
    uint32_t queueFamily = -1;
  };

  // Creates a new command buffer.
  VulkanCmdBuffer* createBuffer( uint32_t queueFamily, bool secondary );

  VulkanDevicePtr _device;
  std::unordered_map<uint32_t, PoolInfo> _pools;
  uint32_t _nextId;
};

class VulkanCmdBuffer
{
public:
  VulkanCmdBuffer( VulkanDevicePtr device, uint32_t id, VkCommandPool pool, 
    uint32_t queueFamily, bool secondary );
};