#ifndef __LAVA_VULKAN_RESOURCE__
#define __LAVA_VULKAN_RESOURCE__

#include "includes.hpp"
#include <memory>

namespace lava
{
  class Device;
  typedef std::shared_ptr<Device> DeviceRef;
  class VulkanResource
  {
  protected:
    VulkanResource( const DeviceRef& device );

    DeviceRef _device;
  };
}

#endif /* __LAVA_VULKAN_RESOURCE__ */