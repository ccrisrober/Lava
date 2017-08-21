#include "VulkanFramebuffer.h"

namespace lava
{
  VulkanFramebuffer::VulkanFramebuffer( VulkanDevicePtr device )
    : VulkanResource( device )
  {
  }

  VulkanFramebuffer::~VulkanFramebuffer( void )
  {
    VkDevice device = _device->getLogical( );
    vkDestroyFramebuffer( device, _framebuffer, nullptr );
    vkDestroyRenderPass( device, _renderPass, nullptr );
  }
}
