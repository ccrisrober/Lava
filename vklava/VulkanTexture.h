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
}

#endif /* __VKLAVA_VULKANTEXTURE__ */