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

#include "Sampler.h"
#include "Device.h"

namespace lava
{
  Sampler::~Sampler( void )
  {
    static_cast< vk::Device >( *_device ).destroySampler( _sampler );
  }
  Sampler::Sampler( const DeviceRef& device, const SamplerStateDesc& desc )
    : VulkanResource( device )
  {
    bool anisotropy = desc.minFilter == FilterOptions::ANISOTROPIC ||
      desc.magFilter == FilterOptions::ANISOTROPIC ||
      desc.mipFilter == FilterOptions::ANISOTROPIC;

    CompareFunction compareFunc = desc.comparisonFunc;

    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.pNext = nullptr;
    samplerInfo.magFilter = getFilter( desc.magFilter );
    samplerInfo.minFilter = getFilter( desc.minFilter );
    samplerInfo.mipmapMode = getMipFilter( desc.mipFilter );
    samplerInfo.addressModeU = getAddressingMode( desc.addressMode.u );
    samplerInfo.addressModeV = getAddressingMode( desc.addressMode.v );
    samplerInfo.addressModeW = getAddressingMode( desc.addressMode.w );
    samplerInfo.mipLodBias = desc.mipmapBias;
    samplerInfo.anisotropyEnable = anisotropy;
    samplerInfo.maxAnisotropy = ( float ) desc.maxAniso;
    samplerInfo.compareEnable = compareFunc != CompareFunction::ALWAYS_PASS;
    samplerInfo.compareOp = getCompareOp( compareFunc );
    samplerInfo.minLod = desc.mipMin;
    samplerInfo.maxLod = desc.mipMax;
    samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    samplerInfo.unnormalizedCoordinates = false;

    _sampler = static_cast< vk::Device >( *_device ).createSampler( samplerInfo );
  }
}