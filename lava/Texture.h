#ifndef __LAVA_TEXTURE__
#define __LAVA_TEXTURE__

#include "includes.hpp"

#include "VulkanResource.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Queue;
  class Sampler;
  class CommandPool;

  class Texture: public VulkanResource
  {
  public:
    static std::shared_ptr<Texture>& create( const DeviceRef& device )
    {
      return std::make_shared< Texture >( device );
    }
    LAVA_API
    Texture( const DeviceRef& device );
    vk::Image image;
    vk::ImageLayout imageLayout;
    vk::DeviceMemory deviceMemory;
    vk::ImageView view;
    vk::Sampler sampler;
    uint32_t width, height;
    uint32_t mipLevels;
    uint32_t layerCount;
    vk::DescriptorImageInfo descriptor;
  };

  class Texture2DArray : public VulkanResource
  {
  public:
    LAVA_API
    // All images on filePaths as same dimensions
    Texture2DArray( const DeviceRef& device, std::vector< std::string >& filePaths,
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue,
      vk::Format format = vk::Format::eR8G8B8A8Unorm,
      bool forceLinear = false );

    LAVA_API
    virtual ~Texture2DArray( void );

    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;
    vk::ImageLayout imageLayout;

    vk::ImageView view;
    vk::Sampler sampler;
  };

  class Texture2D: public VulkanResource
  {
  public:
    LAVA_API
    Texture2D( const DeviceRef& device, const std::string& filename, 
      const std::shared_ptr<CommandPool>& cmdPool, 
      const std::shared_ptr<Queue>& queue,
      vk::Format format = vk::Format::eR8G8B8A8Unorm, 
      bool forceLinear = false );

    LAVA_API
    virtual ~Texture2D( void );

    uint32_t width, height, numChannels;
    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;
    vk::ImageLayout imageLayout;

    vk::ImageView view;
    vk::Sampler sampler;
  };
  class TextureCubemap: public VulkanResource
  {
  public:
    LAVA_API
    TextureCubemap( const DeviceRef& device, 
      const std::array< std::string, 6 >& filePaths, 
      const std::shared_ptr<CommandPool>& cmdPool, 
      const std::shared_ptr<Queue>& queue, 
      vk::Format format = vk::Format::eR8G8B8A8Unorm, 
      bool forceLinear = false );

    LAVA_API
    virtual ~TextureCubemap( void );

    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;
    vk::ImageLayout imageLayout;

    vk::ImageView view;
    vk::Sampler sampler;
  };
  /*class Texture : public VulkanResource
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
  };*/
}

#endif /* __LAVA_TEXTURE__ */