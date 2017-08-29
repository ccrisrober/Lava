#include "VulkanResource.h"

#include "Device.h"

namespace lava
{
  VulkanResource::VulkanResource( const DeviceRef& device )
    : _device( device )
  {
  }
}