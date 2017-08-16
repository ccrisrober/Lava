#include "VulkanResource.h"

namespace vklava
{
  VulkanResource::VulkanResource( VulkanDevicePtr device )
    : _device( device )
  { }

  VulkanResource::~VulkanResource( void )
  { }
}
