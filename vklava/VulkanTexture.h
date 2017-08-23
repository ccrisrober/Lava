#ifndef __VKLAVA_VULKANTEXTURE__
#define __VKLAVA_VULKANTEXTURE__

#include <vulkan/vulkan.h>

#include "VulkanResource.h"

namespace lava
{
  enum TextureType
  {
    /** One dimensional texture. Just a row of pixels. */
    TEX_TYPE_1D = 1,
    /** Two dimensional texture. */
    TEX_TYPE_2D = 2,
    /** Three dimensional texture. */
    TEX_TYPE_3D = 3,
    /** Texture consisting out of six 2D textures describing an inside of a cube. Allows special sampling. */
    TEX_TYPE_CUBE_MAP = 4
  };
  struct VULKAN_IMAGE_DESC
  {
    VkImage image;
    VkDeviceMemory memory;
    VkImageLayout layout;
    TextureType type;
    VkFormat format;
    uint32_t numFaces;
    uint32_t numMiplevels;
    uint32_t usage;
  };

  class VulkanImage : public VulkanResource
  {
  public:
    VulkanImage( VulkanDevicePtr device, const VULKAN_IMAGE_DESC& desc );
    ~VulkanImage( void );
    VkImage getHandle( void ) const
    {
      return _image;
    }
    VkImageView getView( void ) const;
    void* map( uint32_t offset, uint32_t size ) const;
    void unmap( void );
  protected:
    VkImageView createView( void ) const;
    mutable VkImageViewCreateInfo _imageViewCI;
    VkImage _image;
    VkDeviceMemory _memory;
  };





  class Texture//: public VulkanResource
  {
  public:
    VulkanDevicePtr device;
    VkImage image;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    VkImageView view;
    uint32_t width, height;
    VkDescriptorImageInfo descriptor;
    VkSampler sampler;

    void updateDescriptor( void )
    {
      descriptor.sampler = sampler;
      descriptor.imageView = view;
      descriptor.imageLayout = imageLayout;
    }

    void destroy( void )
    {
      vkDestroyImageView( device->getLogical( ), view, nullptr );
      vkDestroyImage( device->getLogical( ), image, nullptr );
      if ( sampler )
      {
        vkDestroySampler( device->getLogical( ), sampler, nullptr );
      }
      device->freeMemory( deviceMemory );
    }
  };

  class Texture2D : public Texture
  {
  public:
    void loadFromFile( const std::string& filename, VkFormat format,
      VulkanDevicePtr device, VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
      VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      bool forceLinear = false );
  };

  class Texture2DArray : public Texture
  {

  };

  class TextureCubeMap : public Texture
  {

  };
}

#endif /* __VKLAVA_VULKANTEXTURE__ */