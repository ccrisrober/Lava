#pragma once

#include <vulkan/vulkan.h>

class VulkanDevice;

// Types of GPU queues.
enum GpuQueueType
{
  /**
  * Queue used for rendering. Allows the use of draw commands, but also all commands supported by compute
  * or upload buffers.
  */
  GPUT_GRAPHICS,
  /** Discrete queue used for compute operations. Allows the use of dispatch and upload commands. */
  GPUT_COMPUTE,
  /** Queue used for memory transfer operations only. No rendering or compute dispatch allowed. */
  GPUT_TRANSFER,
  GPUT_COUNT // Keep at end
};

//#include "VulkanCommandBuffer.h"

class VulkanQueue
{
public:
  VulkanQueue( VulkanDevice& device, VkQueue queue, GpuQueueType type, 
    uint32_t index );
  ~VulkanQueue( void );

  VkQueue getQueue( void ) const
  {
    return _queue;
  }

  /*void submit( VulkanCmdBuffer* cmdBuffer, const std::vector<VulkanSemaphore*>& waitSemaphores,  )
  {
    uint32_t semaphoresCount = waitSemaphores.size( );
    VkSemaphore signalSemaphores[ BS_MAX_VULKAN_CB_DEPENDENCIES + 1 ];
    cmdBuffer->allocateSemaphores( signalSemaphores );

    VkCommandBuffer vkCmdBuffer = cmdBuffer->getHandle( );

    _semaphoresTemp.resize( semaphoresCount + 1 ); // +1 for self semaphore
    prepareSemaphores( waitSemaphores, mSemaphoresTemp.data( ), semaphoresCount );

    VkSubmitInfo submitInfo;
    getSubmitInfo( &vkCmdBuffer, signalSemaphores, BS_MAX_VULKAN_CB_DEPENDENCIES + 1,
      mSemaphoresTemp.data( ), semaphoresCount, submitInfo );

    VkResult result = vkQueueSubmit( _queue, 1, &submitInfo, cmdBuffer->getFence( ) );
    assert( result == VK_SUCCESS );

    cmdBuffer->setIsSubmitted( );
    _lastCommandBuffer = cmdBuffer;
    _lastCBSemaphoreUsed = false;

    _activeSubmissions.push_back( SubmitInfo( cmdBuffer, _nextSubmitIdx++, semaphoresCount, 1 ) );
    _activeBuffers.push( cmdBuffer );
  }*/

protected:
  VulkanDevice& _device;
  VkQueue _queue;
  GpuQueueType _type;
  uint32_t _index;
};

