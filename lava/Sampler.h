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

#ifndef __LAVA_SAMPLER__
#define __LAVA_SAMPLER__

#include <limits>

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace lava
{
  class Sampler : public VulkanResource
  {
  public:
    LAVA_API
    Sampler( const std::shared_ptr<Device>& device, vk::Filter magFilter,
      vk::Filter minFilter, vk::SamplerMipmapMode mipmapMode, 
      vk::SamplerAddressMode addressModeU, vk::SamplerAddressMode addressModeV, 
      vk::SamplerAddressMode addressModeW, float mipLodBias, 
      bool anisotropyEnable, float maxAnisotropy, bool compareEnable,
      vk::CompareOp compareOp, float minLod, float maxLod, 
      vk::BorderColor borderColor, bool unnormalizedCoordinates );
	LAVA_API
	Sampler(const std::shared_ptr<Device>& device, const vk::SamplerCreateInfo& ci);
    LAVA_API
    ~Sampler( void );

    LAVA_API
    inline operator vk::Sampler( void ) const
    {
      return _sampler;
    }

    Sampler( Sampler const& rhs ) = delete;
    Sampler & operator=( Sampler const& rhs ) = delete;

    bool operator==( const Sampler& rhs ) const
    {
      return _sampler == rhs._sampler;
    }
    bool operator!=( const Sampler& rhs ) const
    {
      return _sampler != rhs._sampler;
    }

  private:
    vk::Sampler _sampler;
  };
}

#endif /* __LAVA_SAMPLER__ */