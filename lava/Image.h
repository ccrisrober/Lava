#ifndef __LAVA_IMAGES__
#define __LAVA_IMAGES__

#include <lava/api.h>

#include "includes.hpp"
#include "VulkanResource.h"

#include "noncopyable.hpp"

namespace lava
{
  class Device;
  class ImageView;
  class Image : public VulkanResource, public std::enable_shared_from_this<Image>
  {
  public:
    LAVA_API
    Image( const DeviceRef& device, const vk::Image& image );
    LAVA_API
    Image( const DeviceRef& device, vk::ImageCreateFlags createFlags, 
      vk::ImageType type, vk::Format format, vk::Extent3D extent, 
      uint32_t mipLevels, uint32_t arrayLayers, vk::SampleCountFlagBits samples, 
      vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags, 
      vk::SharingMode sharingMode, const std::vector<uint32_t>& qFamilyIndices,
      vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memPropertyFlags );
    LAVA_API
    virtual ~Image( void );

    LAVA_API
    inline operator vk::Image( void ) const
    {
      return _image;
    }
    LAVA_API
    std::shared_ptr<ImageView> createImageView( vk::ImageViewType viewType, 
      vk::Format format, vk::ComponentMapping components = { 
        vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, 
        vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA 
      }, vk::ImageSubresourceRange subresourceRange = { 
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      }
    );
  private:
    friend class ImageView;
    const DeviceRef& getDevice( void )
    {
      return _device;
    }
  protected:
    uint32_t _arrayLayers;
    vk::Extent3D _extent;
    vk::Format _format;
    vk::Image _image;
    bool _managed;
    vk::MemoryPropertyFlags _memoryPropertyFlags;
    uint32_t _mipLevels;
    std::vector<uint32_t> _queueFamilyIndices;
    vk::SampleCountFlagBits _samples;
    vk::SharingMode _sharingMode;
    vk::ImageTiling _tiling;
    vk::ImageType _type;
  public:
    vk::DeviceMemory imageMemory;
  };

  class ImageView : private NonCopyable<ImageView>
  {
  public:
    LAVA_API
    ImageView( const std::shared_ptr<Image>& image, 
      vk::ImageViewCreateFlags flags, vk::ImageViewType viewType,
      vk::Format format, vk::ComponentMapping components, 
      vk::ImageSubresourceRange subresourceRange );
    LAVA_API
    virtual ~ImageView( void );
    LAVA_API
    inline operator vk::ImageView( void )
    {
      return _imageView;
    }
  protected:
    std::shared_ptr<Image> _image;
    vk::ImageView _imageView;
  };
}

#endif /* __LAVA_IMAGES__ */