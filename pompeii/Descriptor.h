/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#ifndef __POMPEII_DESCRIPTOR__
#define __POMPEII_DESCRIPTOR__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

#include <pompeii/api.h>
#include <pompeii/Buffer.h>
#include <pompeii/Image.h>
#include <pompeii/Sampler.h>

namespace pompeii
{
  struct Descriptor : public vk::DescriptorPoolSize
  {
    Descriptor( vk::DescriptorType type_, 
      uint32_t descriptorCount_ ) noexcept
    {
      this->type = type_;
      this->descriptorCount = descriptorCount_;
    }
  };

  namespace descriptors
  {
    struct Sampler : Descriptor
    {
      Sampler( uint32_t count ) noexcept: 
        Descriptor( vk::DescriptorType::eSampler, count ) { }
    };

    struct CombinedImageSampler : Descriptor
    {
      CombinedImageSampler( uint32_t count ) noexcept
        : Descriptor(vk::DescriptorType::eCombinedImageSampler, count) {}
    };

    struct SampledImage : Descriptor
    {
      SampledImage( uint32_t count ) noexcept
        : Descriptor(vk::DescriptorType::eSampledImage, count) {}
    };

    struct StorageImage : Descriptor
    {
      StorageImage( uint32_t count ) noexcept
        : Descriptor( vk::DescriptorType::eStorageImage, count ) { }
    };

    struct UniformTexelBuffer : Descriptor
    {
      UniformTexelBuffer( uint32_t count ) noexcept
        : Descriptor( vk::DescriptorType::eUniformTexelBuffer, count ) { }
    };

    struct StorageTexelBuffer : Descriptor
    {
      StorageTexelBuffer( uint32_t count ) noexcept
        : Descriptor( vk::DescriptorType::eStorageTexelBuffer, count ) { }
    };

    struct UniformBuffer : Descriptor
    {
      UniformBuffer( uint32_t count ) noexcept
        : Descriptor( vk::DescriptorType::eUniformBuffer, count ) { }
    };

    struct StorageBuffer : Descriptor
    {
      StorageBuffer( uint32_t count ) noexcept
        : Descriptor( vk::DescriptorType::eStorageBuffer, count ) { }
    };

    struct DynamicUniformBuffer : Descriptor
    {
      DynamicUniformBuffer( uint32_t count ) noexcept
        : Descriptor( vk::DescriptorType::eUniformBufferDynamic, count ) { }
    };

    struct DynamicStorageBuffer : Descriptor
    {
      DynamicStorageBuffer( uint32_t count ) noexcept
        : Descriptor( vk::DescriptorType::eStorageBufferDynamic, count ) { }
    };

    struct InputAttachment : Descriptor
    {
      InputAttachment( uint32_t count ) noexcept 
        : Descriptor( vk::DescriptorType::eInputAttachment, count ) { }
    };
  }


  struct DescriptorSetLayoutBinding
  {
    POMPEII_API
    DescriptorSetLayoutBinding( uint32_t binding,
      vk::DescriptorType descriptorType,
      vk::ShaderStageFlags stageFlags, 
      vk::ArrayProxy<const std::shared_ptr<pompeii::Sampler>> immutableSamplers = { }
    );
    POMPEII_API
    DescriptorSetLayoutBinding( DescriptorSetLayoutBinding const& rhs );
    POMPEII_API
    DescriptorSetLayoutBinding& operator=( DescriptorSetLayoutBinding const& rhs );

    uint32_t binding;
    vk::DescriptorType descriptorType;
    vk::ShaderStageFlags stageFlags;
    std::vector<std::shared_ptr<pompeii::Sampler>> immutableSamplers;
  };

  class DescriptorSetLayout : public VulkanResource,
    private NonCopyable<DescriptorSetLayout>
  {
  public:
    POMPEII_API
    DescriptorSetLayout( const std::shared_ptr<Device>& device,
      vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings,
      vk::DescriptorSetLayoutCreateFlags flags = { } );
    POMPEII_API
    ~DescriptorSetLayout( void );

    POMPEII_API
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
    POMPEII_API
    DescriptorPool( const std::shared_ptr<Device>& device, vk::DescriptorPoolCreateFlags flags,
      uint32_t maxSets, vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes );
    POMPEII_API
    ~DescriptorPool( void );

    POMPEII_API
    void reset( void );

    POMPEII_API
    inline operator vk::DescriptorPool( void ) const
    {
      return _descriptorPool;
    }

    POMPEII_API
    static uint32_t max_elem( const std::shared_ptr<Device>& device, 
      vk::DescriptorType type );
  private:
    vk::DescriptorPool  _descriptorPool;
  };

  class DescriptorSet: public VulkanResource, private NonCopyable<DescriptorSet>
  {
  public:
    POMPEII_API
    DescriptorSet( const std::shared_ptr<Device>& device, 
      const std::shared_ptr<DescriptorPool>& descriptorPool,
      const std::shared_ptr<DescriptorSetLayout>& layout);
    POMPEII_API
    ~DescriptorSet( void );

    POMPEII_API
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
    POMPEII_API
    DescriptorBufferInfo( void )
    {
    }
    POMPEII_API
    DescriptorBufferInfo( const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset, vk::DeviceSize range );
    POMPEII_API
    DescriptorBufferInfo( const DescriptorBufferInfo& rhs );
    POMPEII_API
    DescriptorBufferInfo& operator=( const DescriptorBufferInfo& rhs );

    std::shared_ptr<Buffer> buffer;
    vk::DeviceSize offset;
    vk::DeviceSize range;
  };

  struct DescriptorImageInfo
  {
    POMPEII_API
    DescriptorImageInfo( void )
    {
    }
    POMPEII_API
    DescriptorImageInfo( vk::ImageLayout imageLayout,
      const std::shared_ptr<ImageView>& imageView,
      const std::shared_ptr<Sampler>& sampler );
    POMPEII_API
    DescriptorImageInfo( const DescriptorImageInfo& rhs );
    POMPEII_API
    DescriptorImageInfo& operator=( const DescriptorImageInfo& rhs );

    vk::ImageLayout imageLayout;
    std::shared_ptr<ImageView> imageView;
    std::shared_ptr<Sampler> sampler; 
  };

  struct WriteDescriptorSet
  {
    POMPEII_API
    WriteDescriptorSet( const std::shared_ptr<DescriptorSet>& dstSet,
      uint32_t dstBinding, uint32_t dstArrayElement,
      vk::DescriptorType descriptorType, uint32_t descriptorCount,
      vk::Optional<const DescriptorImageInfo> imageInfo = nullptr,
      vk::Optional<const DescriptorBufferInfo> bufferInfo = nullptr,
      const std::shared_ptr<pompeii::BufferView>& bufferView = { }
    );
    POMPEII_API
    WriteDescriptorSet( const WriteDescriptorSet& rhs );
    POMPEII_API
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
    POMPEII_API
    CopyDescriptorSet( const std::shared_ptr<DescriptorSet>& srcSet,
      uint32_t srcBinding, uint32_t srcArrayElement,
      const std::shared_ptr<DescriptorSet>& dstSet, uint32_t dstBinding,
      uint32_t dstArrayElement, uint32_t descriptorCount );
    POMPEII_API
    CopyDescriptorSet( const CopyDescriptorSet& cds );
    POMPEII_API
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

#endif /* __POMPEII_DESCRIPTOR__ */