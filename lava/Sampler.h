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
    ~Sampler( void );

    LAVA_API
    inline operator vk::Sampler( void ) const
    {
      return _sampler;
    }

    Sampler( Sampler const& rhs ) = delete;
    Sampler & operator=( Sampler const& rhs ) = delete;

  private:
    vk::Sampler _sampler;
  };
}

#endif /* __LAVA_SAMPLER__ */