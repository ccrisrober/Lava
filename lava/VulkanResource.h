#ifndef __LAVA_VULKAN_RESOURCE__
#define __LAVA_VULKAN_RESOURCE__

#include "includes.hpp"
#include <memory>

#include <lava/api.h>

namespace lava
{
  class Device;
  typedef std::shared_ptr<Device> DeviceRef;
  class VulkanResource
  {
  public:
    LAVA_API
    const DeviceRef getDevice( void )
    {
      return _device;
    }
  protected:
    VulkanResource( const DeviceRef& device );

    DeviceRef _device;
  };
}

#endif /* __LAVA_VULKAN_RESOURCE__ */