/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __LAVA_IMAGES__
#define __LAVA_IMAGES__

#include <lava/api.h>

#include "includes.hpp"
#include "VulkanResource.h"

#include "noncopyable.hpp"

namespace lava
{
  class ImageView;
  class Image 
    : public VulkanResource
    , public std::enable_shared_from_this<Image>
  {
  public:
    LAVA_API
    Image( const std::shared_ptr<Device>& device, const vk::Image& image );
    LAVA_API
    Image( const std::shared_ptr<Device>& device, vk::ImageCreateFlags createFlags, 
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
    std::shared_ptr<ImageView> createImageView( uint32_t mipLevelCount, 
      vk::ImageAspectFlagBits aspect, vk::ComponentMapping swizzle );
    LAVA_API
    std::shared_ptr<ImageView> createImageView( vk::ImageViewType viewType, 
      vk::Format format, vk::ComponentMapping components = { 
        vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, 
        vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA 
      }, vk::ImageSubresourceRange subresourceRange = { 
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      }
    );
    LAVA_API
    std::shared_ptr<ImageView> createImageView( vk::ImageViewType viewType,
      vk::Format format, vk::ImageAspectFlags aspect );
    LAVA_API
    inline vk::Format format( void ) const
    {
      return _format;
    }
    LAVA_API
    inline vk::Extent3D extent( void ) const
    {
      return _extent;
    }
    bool operator==( const Image& rhs ) const
    {
      return _image == rhs._image && _format == rhs._format;
    }
    bool operator!=( const Image& rhs ) const
    {
      return _image != rhs._image || _format != rhs._format;
    }
  private:
    friend class ImageView;
    const std::shared_ptr<Device>& getDevice( void )
    {
      return _device;
    }
  public:
    vk::ImageLayout layout; // TODO: MOVE
  protected:
    vk::ImageCreateFlags _createFlags;
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

  private:
    bool depth( const vk::Format& format ) const noexcept
    {
      switch ( format )
      {
        case vk::Format::eD16Unorm:
        case vk::Format::eD32Sfloat:
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
          return true;
        default:
          return false;
      }
    }
    bool depthStencil( const vk::Format& format ) const noexcept
    {
      switch ( format )
      {
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
          return true;
        default:
          return false;
      }
    }
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
    bool operator==( const ImageView& rhs ) const
    {
      return _imageView == rhs._imageView;
    }
    bool operator!=( const ImageView& rhs ) const
    {
      return _imageView != rhs._imageView;
    }
  protected:
    std::shared_ptr<Image> _image;
    vk::ImageView _imageView;
  };
}

#endif /* __LAVA_IMAGES__ */