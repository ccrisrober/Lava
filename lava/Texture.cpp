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
