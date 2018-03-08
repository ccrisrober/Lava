/**
 * Copyright (c) 2017, Lava
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

#include "Image.h"
#include "Device.h"
#include "PhysicalDevice.h"

namespace lava
{
  Image::Image( const std::shared_ptr<Device>& device, const vk::Image& image )
    : VulkanResource( device )
    , _image( image )
    , _managed( false )
  {
  }
  Image::Image( const std::shared_ptr<Device>& device, 
    vk::ImageCreateFlags createFlags, vk::ImageType type,
    vk::Format format, vk::Extent3D extent, uint32_t mipLevels, 
    uint32_t arrayLayers, vk::SampleCountFlagBits samples,
    vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags, 
    vk::SharingMode sharingMode, const std::vector<uint32_t>& qFamilyIndices,
    vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memPropFlags )
    : VulkanResource( device )
    , _arrayLayers( arrayLayers )
    , _extent( extent )
    , _format( format )
    , _managed( true )
    , _memoryPropertyFlags( memPropFlags )
    , _mipLevels( mipLevels )
    , _queueFamilyIndices( qFamilyIndices )
    , _samples( samples )
    , _sharingMode( sharingMode )
    , _tiling( tiling )
    , _type( type )
  {
    vk::ImageCreateInfo createInfo( createFlags, _type, _format, _extent, 
      _mipLevels, _arrayLayers, _samples, _tiling, usageFlags, _sharingMode,
      _queueFamilyIndices.size( ), _queueFamilyIndices.data( ), initialLayout );
    _image = static_cast< vk::Device >( *_device ).createImage( createInfo );

    auto memReqs = static_cast< vk::Device >( *_device )
                        .getImageMemoryRequirements( _image );
    uint32_t memoryTypeIndex = findMemoryType( _device->getPhysicalDevice( )
                        ->getMemoryProperties( ), memReqs.memoryTypeBits, 
                        _memoryPropertyFlags );
    assert( memoryTypeIndex != uint32_t( -1 ) );
    imageMemory = _device->allocateMemReqMemory( memReqs, _memoryPropertyFlags );
    vk::Device( *_device ).bindImageMemory( _image, imageMemory, 0 );
  }
  Image::~Image( void )
  {
    if ( _managed )
    {
      static_cast< vk::Device >( *_device ).destroyImage( _image );
      _device->freeMemory( imageMemory );
      std::cout << "Image destroyed" << std::endl;
    }
  }

  std::shared_ptr<ImageView> Image::createImageView( vk::ImageViewType viewType, 
    vk::Format format, vk::ComponentMapping components, 
    vk::ImageSubresourceRange isrr )
  {
    return std::make_shared< ImageView >( shared_from_this( ), 
      vk::ImageViewCreateFlags( ), viewType,
      format, components, isrr );
  }

  ImageView::ImageView( const std::shared_ptr<Image>& image, 
    vk::ImageViewCreateFlags flags, vk::ImageViewType viewType,
    vk::Format format, vk::ComponentMapping components, 
    vk::ImageSubresourceRange isrr )
    : _image( image )
  {
    vk::ImageViewCreateInfo ivci( flags, *image, viewType, format, components, isrr );

    _imageView = static_cast< vk::Device >( *_image->getDevice( ) )
          .createImageView( ivci );
  }
  ImageView::~ImageView( void )
  {
    static_cast< vk::Device >( *_image->getDevice( ) )
          .destroyImageView( _imageView );
    std::cout << "Destroyed ImageView" << std::endl;
  }
}