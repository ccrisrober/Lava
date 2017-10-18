#include "Texture.h"

#include "Device.h"
#include "PhysicalDevice.h"

#include "utils.hpp"

namespace lava
{
  Texture::Texture(  const DeviceRef& device )
    : VulkanResource( device )
  {

  }
}
