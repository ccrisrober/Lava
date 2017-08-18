#include "VulkanFramebuffer.h"

namespace lava
{
  VulkanFramebuffer::VulkanFramebuffer( VulkanDevicePtr device )
    : VulkanResource( device )
  { }

  VulkanFramebuffer::~VulkanFramebuffer( void )
  { }
}
