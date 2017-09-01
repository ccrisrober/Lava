#ifndef __LAVA_TEXTURE__
#define __LAVA_TEXTURE__

#include "includes.hpp"

#include "VulkanResource.h"

namespace lava
{
  class Device;
  class Texture : public VulkanResource
  {
  public:
    Texture( const DeviceRef& device );
    vk::Image image;
    vk::ImageLayout imageLayout;
    vk::DeviceMemory memory;
    vk::ImageView view;
    uint32_t width, height;
    uint32_t mipLevels;
    uint32_t layerCount;

    vk::DescriptorImageInfo descriptor;

    vk::Sampler sampler;

    void updateDescriptor( );
    void destroy( );
  };

  class Texture2D : public Texture
  {
  public:
    void loadFromFile( const DeviceRef& device, const std::string& filename, 
      vk::Format format, vk::Queue copyQueue, 
      vk::ImageUsageFlagBits = vk::ImageUsageFlagBits::eSampled,
      vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal, 
      bool forceLinear = false );
  };
}

#endif /* __LAVA_TEXTURE__ */