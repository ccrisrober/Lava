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

#ifndef __LAVA_DESCRIPTOR__
#define __LAVA_DESCRIPTOR__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

#include <lava/api.h>
#include <lava/Buffer.h>
#include <lava/Image.h>
#include <lava/Sampler.h>

namespace lava
{
  struct DescriptorSetLayoutBinding
  {
    LAVA_API
    DescriptorSetLayoutBinding( uint32_t binding,
      vk::DescriptorType descriptorType,
      vk::ShaderStageFlags stageFlags, 
      vk::ArrayProxy<const std::shared_ptr<lava::Sampler>> immutableSamplers = { }
    );
    LAVA_API
    DescriptorSetLayoutBinding( DescriptorSetLayoutBinding const& rhs );
    LAVA_API
    DescriptorSetLayoutBinding& operator=( DescriptorSetLayoutBinding const& rhs );

    uint32_t binding;
    vk::DescriptorType descriptorType;
    vk::ShaderStageFlags stageFlags;
    std::vector<std::shared_ptr<lava::Sampler>> immutableSamplers;
  };

  class DescriptorSetLayout : public VulkanResource,
    private NonCopyable<DescriptorSetLayout>
  {
  public:
    LAVA_API
    DescriptorSetLayout( const std::shared_ptr<Device>& device,
      vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings,
      vk::DescriptorSetLayoutCreateFlags flags = { } );
    LAVA_API
    ~DescriptorSetLayout( void );

    LAVA_API
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
    DescriptorPool( const std::shared_ptr<Device>& device, vk::DescriptorPoolCreateFlags flags,
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
    DescriptorSet( const std::shared_ptr<Device>& device, 
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

  struct DescriptorBufferInfo
  {
    LAVA_API
    DescriptorBufferInfo( void )
    {
    }
    LAVA_API
    DescriptorBufferInfo( const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset, vk::DeviceSize range );
    LAVA_API
    DescriptorBufferInfo( const DescriptorBufferInfo& rhs );
    LAVA_API
    DescriptorBufferInfo& operator=( const DescriptorBufferInfo& rhs );

    std::shared_ptr<Buffer> buffer;
    vk::DeviceSize offset;
    vk::DeviceSize range;
  };

  struct DescriptorImageInfo
  {
    LAVA_API
    DescriptorImageInfo( void )
    {
    }
    LAVA_API
    DescriptorImageInfo( vk::ImageLayout imageLayout,
      const std::shared_ptr<ImageView>& imageView,
      const std::shared_ptr<Sampler>& sampler );
    LAVA_API
    DescriptorImageInfo( const DescriptorImageInfo& rhs );
    LAVA_API
    DescriptorImageInfo& operator=( const DescriptorImageInfo& rhs );

    vk::ImageLayout imageLayout;
    std::shared_ptr<ImageView> imageView;
    std::shared_ptr<Sampler> sampler; 
  };

  struct WriteDescriptorSet
  {
    LAVA_API
    WriteDescriptorSet( const std::shared_ptr<DescriptorSet>& dstSet,
      uint32_t dstBinding, uint32_t dstArrayElement,
      vk::DescriptorType descriptorType, uint32_t descriptorCount,
      vk::Optional<const DescriptorImageInfo> imageInfo = nullptr,
      vk::Optional<const DescriptorBufferInfo> bufferInfo = nullptr,
      const std::shared_ptr<lava::BufferView>& bufferView = { }
    );
    LAVA_API
    WriteDescriptorSet( const WriteDescriptorSet& rhs );
    LAVA_API
    WriteDescriptorSet& operator=( const WriteDescriptorSet& rhs );

    std::shared_ptr<DescriptorSet> dstSet;
    uint32_t dstBinding;
    uint32_t dstArrayElement;
    vk::DescriptorType descriptorType;
    uint32_t descriptorCount;
    std::unique_ptr<DescriptorImageInfo> imageInfo;
    std::unique_ptr<DescriptorBufferInfo> bufferInfo;
    std::shared_ptr<BufferView> texelBufferView;
  };

  struct CopyDescriptorSet
  {
    LAVA_API
    CopyDescriptorSet( const std::shared_ptr<DescriptorSet>& srcSet,
      uint32_t srcBinding, uint32_t srcArrayElement,
      const std::shared_ptr<DescriptorSet>& dstSet, uint32_t dstBinding,
      uint32_t dstArrayElement, uint32_t descriptorCount );
    LAVA_API
    CopyDescriptorSet( const CopyDescriptorSet& cds );
    LAVA_API
    CopyDescriptorSet& operator=( const CopyDescriptorSet& cds );

    std::shared_ptr<DescriptorSet> srcSet;
    uint32_t srcBinding;
    uint32_t srcArrayElement;
    std::shared_ptr<DescriptorSet> dstSet;
    uint32_t dstBinding;
    uint32_t dstArrayElement;
    uint32_t descriptorCount;
  };
}

#endif /* __LAVA_DESCRIPTOR__ */