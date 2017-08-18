#include "VulkanResource.h"

namespace lava
{
  VulkanResource::VulkanResource( VulkanDevicePtr device )
    : _device( device )
  { }

  VulkanResource::~VulkanResource( void )
  { }
}
