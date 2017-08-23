#include "VulkanSampler.h"
#include "VulkanRenderAPI.h"

namespace lava
{
  VulkanSampler::~VulkanSampler( void )
  {
    vkDestroySampler( _device->getLogical( ), _sampler, nullptr );
  }
  VulkanSamplerState::VulkanSamplerState( const SamplerStateDesc& desc )
    : _desc( desc )
  {
    bool anisotropy = _desc.minFilter == FilterOptions::ANISOTROPIC ||
      _desc.magFilter == FilterOptions::ANISOTROPIC ||
      _desc.mipFilter == FilterOptions::ANISOTROPIC;

    CompareFunction compareFunc = _desc.comparisonFunc;
    

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.flags = 0;
    samplerInfo.pNext = nullptr;
    samplerInfo.magFilter = getFilter( _desc.magFilter );
    samplerInfo.minFilter = getFilter( _desc.minFilter );
    samplerInfo.mipmapMode = getMipFilter( _desc.mipFilter );
    samplerInfo.addressModeU = getAddressingMode( _desc.addressMode.u );
    samplerInfo.addressModeV = getAddressingMode( _desc.addressMode.v );
    samplerInfo.addressModeW = getAddressingMode( _desc.addressMode.w );
    samplerInfo.mipLodBias = _desc.mipmapBias;
    samplerInfo.anisotropyEnable = anisotropy;
    samplerInfo.maxAnisotropy = (float)_desc.maxAniso;
    samplerInfo.compareEnable = compareFunc != CompareFunction::ALWAYS_PASS;
    samplerInfo.compareOp = getCompareOp( compareFunc );
    samplerInfo.minLod = _desc.mipMin;
    samplerInfo.maxLod = _desc.mipMax;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = false;

    auto api = VulkanRenderAPI::getInstance( );

    VulkanDevicePtr device = api->_getPresentDevice( );

    VkSampler sampler;
    VkResult result = vkCreateSampler( device->getLogical( ), &samplerInfo, 
      nullptr, &sampler);
    assert(result == VK_SUCCESS);

    _sampler = new VulkanSampler( device, sampler );
  }
  VulkanSamplerState::~VulkanSamplerState( void )
  {
    delete _sampler;
  }
}