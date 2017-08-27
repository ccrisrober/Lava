#include "Sampler.h"
#include "Device.h"

namespace lava
{
	Sampler::~Sampler( void )
	{
		static_cast<vk::Device>(*_device).destroySampler(_sampler);
	}
	Sampler::Sampler(std::shared_ptr<Device> const & device, const SamplerStateDesc& desc)
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
    samplerInfo.maxAnisotropy = (float)desc.maxAniso;
    samplerInfo.compareEnable = compareFunc != CompareFunction::ALWAYS_PASS;
    samplerInfo.compareOp = getCompareOp( compareFunc );
    samplerInfo.minLod = desc.mipMin;
    samplerInfo.maxLod = desc.mipMax;
    samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    samplerInfo.unnormalizedCoordinates = false;

	_sampler = static_cast<vk::Device>(*_device).createSampler(samplerInfo);
  }
}