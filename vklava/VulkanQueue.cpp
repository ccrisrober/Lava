#include "VulkanQueue.h"
#include "VulkanDevice.h"

VulkanQueue::VulkanQueue( VulkanDevice& device, VkQueue queue, 
  GpuQueueType type, uint32_t index )
  : _device( device )
  , _queue( queue )
  , _type( type )
  , _index( index )
{
}


VulkanQueue::~VulkanQueue( void )
{ }
