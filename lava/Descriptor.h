#ifndef __LAVA_DESCRIPTOR__
#define __LAVA_DESCRIPTOR__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

#include <lava/api.h>

namespace lava
{
  class Device;
  struct DescriptorSetLayoutBinding
  {
    DescriptorSetLayoutBinding( uint32_t binding,
      vk::DescriptorType descriptorType,
      vk::ShaderStageFlags stageFlags );
    DescriptorSetLayoutBinding( DescriptorSetLayoutBinding const& rhs );
    DescriptorSetLayoutBinding & operator=( DescriptorSetLayoutBinding const& rhs );

    uint32_t binding;
    vk::DescriptorType descriptorType;
    vk::ShaderStageFlags stageFlags;
  };

  class DescriptorSetLayout : public VulkanResource,
    private NonCopyable<DescriptorSetLayout>
  {
  public:
    DescriptorSetLayout( const DeviceRef& device,
      vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings );
    ~DescriptorSetLayout( );

    inline operator vk::DescriptorSetLayout( void ) const
    {
      return _descriptorSetLayout;
    }
  private:
    vk::DescriptorSetLayout _descriptorSetLayout;
    std::vector<DescriptorSetLayoutBinding> _bindings;
  };
  class DescriptorPool : public VulkanResource, private NonCopyable<DescriptorPool>
  {
  public:
    LAVA_API
    DescriptorPool( const DeviceRef& device, vk::DescriptorPoolCreateFlags flags,
      uint32_t maxSets, vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes );
    LAVA_API
    ~DescriptorPool( void );

    LAVA_API
    void reset( void );

    LAVA_API
    inline operator vk::DescriptorPool( void ) const
    {
      return _descriptorPool;
    }

  private:
    vk::DescriptorPool  _descriptorPool;
  };

  class DescriptorSet: public VulkanResource, private NonCopyable<DescriptorSet>
  {
  public:
    LAVA_API
    DescriptorSet( const DeviceRef& device, 
      const std::shared_ptr<DescriptorPool>& descriptorPool,
      const std::shared_ptr<DescriptorSetLayout>& layout);
    LAVA_API
    ~DescriptorSet( void );

    LAVA_API
    inline operator vk::DescriptorSet( void ) const
    {
      return _descriptorSet;
    }
  protected:
    vk::DescriptorSet _descriptorSet;
    std::shared_ptr< DescriptorPool > _descriptorPool;
  };
}

#endif /* __LAVA_DESCRIPTOR__ */