#include "Image.h"
#include "Device.h"
#include "PhysicalDevice.h"

namespace lava
{
  Image::Image( const DeviceRef& device, const vk::Image& image )
    : VulkanResource( device )
    , _image( image )
    , _managed( false )
  {
  }
  Image::Image( const DeviceRef& device, vk::ImageCreateFlags createFlags, vk::ImageType type,
    vk::Format format, vk::Extent3D extent, uint32_t mipLevels, uint32_t arrayLayers, vk::SampleCountFlagBits samples,
    vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags, vk::SharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices,
    vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryPropertyFlags )
    : VulkanResource( device )
    , _arrayLayers( arrayLayers )
    , _extent( extent )
    , _format( format )
    , _managed( true )
    , _memoryPropertyFlags( memoryPropertyFlags )
    , _mipLevels( mipLevels )
    , _queueFamilyIndices( queueFamilyIndices )
    , _samples( samples )
    , _sharingMode( sharingMode )
    , _tiling( tiling )
    , _type( type )
  {
    vk::ImageCreateInfo createInfo( createFlags, _type, _format, _extent, _mipLevels, _arrayLayers, _samples, _tiling, usageFlags, _sharingMode,
      _queueFamilyIndices.size( ), _queueFamilyIndices.data( ), initialLayout );
    _image = static_cast< vk::Device >( *_device ).createImage( createInfo );

    vk::MemoryRequirements memReqs = static_cast< vk::Device >( *_device ).getImageMemoryRequirements( _image );
    auto mem = _device->allocateMemReqMemory( memReqs, _memoryPropertyFlags );
    vk::Device( *_device ).bindImageMemory( _image, mem, 0 );
  }
  Image::~Image( )
  {
    if ( _managed )
    {
      static_cast< vk::Device >( *_device ).destroyImage( _image );
    }
  }

  std::shared_ptr<ImageView> Image::createImageView( vk::ImageViewType viewType, vk::Format format,
    vk::ComponentMapping components, vk::ImageSubresourceRange subresourceRange )
  {
    return std::make_shared<ImageView>( shared_from_this( ), vk::ImageViewCreateFlags( ), viewType,
      format, components, subresourceRange );
  }

  ImageView::ImageView( const std::shared_ptr<Image>& image, vk::ImageViewCreateFlags flags, vk::ImageViewType viewType,
    vk::Format format, vk::ComponentMapping components, vk::ImageSubresourceRange subresourceRange )
    : _image( image )
  {
    vk::ImageViewCreateInfo ivci( flags, *image, viewType, format, components, subresourceRange );

    _imageView = static_cast< vk::Device >( *_image->getDevice( ) ).createImageView( ivci );
  }
  ImageView::~ImageView( )
  {
    static_cast< vk::Device >( *_image->getDevice( ) ).destroyImageView( _imageView );
  }
}