#ifndef __VKLAVA_VULKANRESOURCE__
#define __VKLAVA_VULKANRESOURCE__

#include "VulkanDevice.h"

namespace lava
{
  class VulkanResource
  {
  public:
    VulkanResource( VulkanDevicePtr device );
    ~VulkanResource( void );
  protected:
    VulkanDevicePtr _device;
  };
}

#endif /* __VKLAVA_VULKANRESOURCE__ */