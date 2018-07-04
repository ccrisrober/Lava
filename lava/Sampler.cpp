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

#include "Sampler.h"
#include "Log.h"

namespace lava
{
  Sampler::Sampler( const std::shared_ptr<Device>& device, vk::Filter magFilter,
    vk::Filter minFilter, vk::SamplerMipmapMode mipmapMode, 
    vk::SamplerAddressMode addressModeU, vk::SamplerAddressMode addressModeV, 
    vk::SamplerAddressMode addressModeW, float mipLodBias, bool anisotropyEnable, 
    float maxAnisotropy, bool compareEnable, vk::CompareOp compareOp, 
    float minLod, float maxLod, vk::BorderColor borderColor, 
    bool unnormalizedCoordinates )
    : VulkanResource( device )
  {
    if ( anisotropyEnable && maxAnisotropy <= 0.0f )
    {
      Log::error( "Can't create a sampler with enabled anisotropy and 0.0f "
        "for max value. Disabling anisotropy" );
      maxAnisotropy = false;
    }

    vk::SamplerCreateInfo csci( { }, magFilter, minFilter, mipmapMode, 
      addressModeU, addressModeV, addressModeW, mipLodBias, anisotropyEnable, 
      maxAnisotropy, compareEnable, compareOp, minLod, maxLod,
      borderColor, unnormalizedCoordinates );
    _sampler = static_cast<vk::Device>( *_device ).createSampler( csci );
  }

  Sampler::Sampler(const std::shared_ptr<Device>& device, 
	const vk::SamplerCreateInfo& ci)
	: Sampler( device, ci.magFilter, ci.minFilter, ci.mipmapMode, 
	  ci.addressModeU, ci.addressModeV, ci.addressModeW,
	  ci.mipLodBias, ci.anisotropyEnable, ci.maxAnisotropy, ci.compareEnable,
	  ci.compareOp, ci.minLod, ci.maxLod, ci.borderColor, 
	  ci.unnormalizedCoordinates)
  {
  }

  Sampler::~Sampler( void )
  {
    static_cast<vk::Device>( *_device ).destroySampler( _sampler );
  }
}