#include "VulkanFramebuffer.h"

namespace vklava
{
  VulkanFramebuffer::VulkanFramebuffer( VulkanDevicePtr device )
    : VulkanResource( device )
  { }

  VulkanFramebuffer::~VulkanFramebuffer( void )
  { }
}
