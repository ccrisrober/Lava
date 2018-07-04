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

#ifndef __LAVA_TEXTURE__
#define __LAVA_TEXTURE__

#include "includes.hpp"

#include "VulkanResource.h"
#include "Descriptor.h"
#include "Image.h"
#include "Sampler.h"

#include <lava/api.h>

namespace lava
{
  class Texture: public VulkanResource
  {
  public:
    LAVA_API
    Texture( const std::shared_ptr<Device>& device );
    LAVA_API
    virtual ~Texture( void );
    LAVA_API
    void updateDescriptor( void );

    bool operator==( const Texture& rhs ) const
    {
      return image == rhs.image && view == rhs.view && sampler == rhs.sampler;
    }
    bool operator!=( const Texture& rhs ) const
    {
      return image != rhs.image
        || view != rhs.view
        || sampler != rhs.sampler;
    }

    std::shared_ptr<Image> image;
    std::shared_ptr<ImageView> view;
    std::shared_ptr<Sampler> sampler;
    vk::ImageLayout imageLayout;
    uint32_t width, height;
    uint32_t mipLevels;
    uint32_t layerCount;
    DescriptorImageInfo descriptor;
  };
}

#endif /* __LAVA_TEXTURE__ */