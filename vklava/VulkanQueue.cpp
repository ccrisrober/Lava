#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include <assert.h>

namespace vklava
{
  VulkanQueue::VulkanQueue( VulkanDevice& device, VkQueue queue,
    GpuQueueType type, uint32_t index )
    : _device( device )
    , _queue( queue )
    , _type( type )
    , _index( index )
  { }

  VulkanQueue::~VulkanQueue( void )
  { }

  void VulkanQueue::waitIdle( void ) const
  {
    VkResult result = vkQueueWaitIdle( _queue );
    assert( result == VK_SUCCESS );
  }
}
