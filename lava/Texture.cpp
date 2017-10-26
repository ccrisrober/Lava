#include "Texture.h"

#include "Device.h"
#include "PhysicalDevice.h"

#include "utils.hpp"

namespace lava
{
  Texture::Texture(  const DeviceRef& device )
    : VulkanResource( device )
  {

  }
  Texture::~Texture( void )
  {
    vk::Device device = static_cast< vk::Device >( *_device );
    device.destroyImageView( view );
    device.destroyImage( image );
    if ( sampler )
    {
      device.destroySampler( sampler );
    }
    _device->freeMemory( deviceMemory );
  }
  void Texture::updateDescriptor( void )
  {
    descriptor.imageLayout = imageLayout;
    descriptor.imageView = std::make_shared< vk::ImageView>( view );
    descriptor.sampler = std::make_shared< vk::Sampler>( sampler );
  }
}
