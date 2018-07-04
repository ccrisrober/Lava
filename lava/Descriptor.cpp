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

#include "Descriptor.h"

#include "Device.h"
#include "PhysicalDevice.h"

namespace lava
{
  DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(
    uint32_t binding_, vk::DescriptorType descriptorType_,
    vk::ShaderStageFlags stageFlags_,
    vk::ArrayProxy<const std::shared_ptr<lava::Sampler>> iss )
    : binding( binding_ )
    , descriptorType( descriptorType_ )
    , stageFlags( stageFlags_ )
    , immutableSamplers( iss.begin( ), iss.end( ) )
  {
  }

  DescriptorSetLayoutBinding::DescriptorSetLayoutBinding( 
    DescriptorSetLayoutBinding const& rhs )
    : DescriptorSetLayoutBinding( rhs.binding, rhs.descriptorType, 
      rhs.stageFlags, rhs.immutableSamplers)
  {
  }

  DescriptorSetLayoutBinding& 
  DescriptorSetLayoutBinding::operator=( DescriptorSetLayoutBinding const& rhs )
  {
    binding = rhs.binding;
    descriptorType = rhs.descriptorType;
    stageFlags = rhs.stageFlags;
    immutableSamplers = rhs.immutableSamplers;
    return *this;
  }

  DescriptorBufferInfo::DescriptorBufferInfo( 
    const std::shared_ptr<Buffer>& buffer_,  vk::DeviceSize offset_, 
    vk::DeviceSize range_ )
    : buffer( buffer_ )
    , offset( offset_ )
    , range( range_ )
  {
  }
  DescriptorBufferInfo::DescriptorBufferInfo( const DescriptorBufferInfo& rhs )
    : DescriptorBufferInfo( rhs.buffer, rhs.offset, rhs.range )
  {
  }
  DescriptorBufferInfo& DescriptorBufferInfo::operator=( 
    const DescriptorBufferInfo& rhs )
  {
    buffer = rhs.buffer;
    offset = rhs.offset;
    range = rhs.range;
    return *this;
  }



  DescriptorSetLayout::DescriptorSetLayout( const std::shared_ptr<Device>& device,
    vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings,
    vk::DescriptorSetLayoutCreateFlags flags )
    : VulkanResource( device )
    , _bindings( bindings.begin( ), bindings.end( ) )
  {
    std::vector< std::vector< vk::Sampler > > samplers;
    samplers.reserve( _bindings.size( ) );

    std::vector<vk::DescriptorSetLayoutBinding> dslb;
    dslb.reserve( _bindings.size( ) );

    for ( const auto& bind : _bindings )
    {
      samplers.push_back(std::vector< vk::Sampler> ( ) );
      samplers.back( ).reserve( bind.immutableSamplers.size( ) );

      for( const auto& samp: bind.immutableSamplers )
      {
        samplers.back( ).push_back( static_cast< vk::Sampler > ( *samp ) );
      }

      uint32_t dscptCount = 1;
      if (!samplers.back( ).empty( ) && bind.descriptorType == 
        vk::DescriptorType::eSampler )
      {
        dscptCount = samplers.back( ).size( );
      }

      dslb.push_back( 
        vk::DescriptorSetLayoutBinding( 
          bind.binding, 
          bind.descriptorType, 
          dscptCount, 
          bind.stageFlags, 
          samplers.back( ).data( )
        )
      );
    }
    vk::DescriptorSetLayoutCreateInfo dci(
      flags,
      dslb.size( ),
      dslb.data( )
    );
    _descriptorSetLayout = static_cast< vk::Device >( *_device )
      .createDescriptorSetLayout( dci );
  }

  DescriptorSetLayout::~DescriptorSetLayout( void )
  {
    static_cast< vk::Device >( *_device ).destroyDescriptorSetLayout( 
      _descriptorSetLayout );
  }


  DescriptorPool::DescriptorPool( const std::shared_ptr<Device>& device,
    vk::DescriptorPoolCreateFlags flags, uint32_t maxSets,
    vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes )
    : VulkanResource( device )
  {
    // we need to set this bit to enable free on their descriptor sets
    flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    vk::DescriptorPoolCreateInfo dci(
      flags,
      maxSets,
      poolSizes.size( ),
      poolSizes.data( )
    );

    _descriptorPool = static_cast< vk::Device >( *_device )
      .createDescriptorPool( dci );
  }

  DescriptorPool::~DescriptorPool( void )
  {
    static_cast< vk::Device >( *_device ).destroyDescriptorPool( _descriptorPool );
  }

  void DescriptorPool::reset( void )
  {
    static_cast< vk::Device >( *_device ).resetDescriptorPool( _descriptorPool );
  }

  uint32_t DescriptorPool::max_elem( const std::shared_ptr<Device>& device, 
    vk::DescriptorType type )
  {
    auto limits = device->getPhysicalDevice( )->getDeviceProperties( ).limits;
    switch ( type )
    {
      case vk::DescriptorType::eUniformBuffer:
        return limits.maxPerStageDescriptorUniformBuffers;
      case vk::DescriptorType::eUniformBufferDynamic:
        return std::min( limits.maxDescriptorSetUniformBuffersDynamic,
          limits.maxPerStageDescriptorUniformBuffers );

      case vk::DescriptorType::eStorageBuffer:
        return limits.maxPerStageDescriptorStorageBuffers;
      case vk::DescriptorType::eStorageBufferDynamic:
        return std::min( limits.maxDescriptorSetStorageBuffersDynamic,
          limits.maxPerStageDescriptorStorageBuffers );

      case vk::DescriptorType::eCombinedImageSampler:
      case vk::DescriptorType::eSampledImage:
      case vk::DescriptorType::eUniformTexelBuffer:
        return limits.maxPerStageDescriptorSampledImages;

      case vk::DescriptorType::eStorageTexelBuffer:
      case vk::DescriptorType::eStorageImage:
        return limits.maxPerStageDescriptorStorageImages;

      default:
        break;
    };
    throw "Invalid binding type." ;
  }

  DescriptorSet::DescriptorSet( const std::shared_ptr<Device>& device, 
    const std::shared_ptr<DescriptorPool>& descriptorPool,
    const std::shared_ptr<DescriptorSetLayout>& layout)
    : VulkanResource( device )
    , _descriptorPool( descriptorPool )
  {
    vk::DescriptorSetLayout lay = *layout;
    vk::DescriptorSetAllocateInfo dci( *_descriptorPool, 1, &lay );

    std::vector< vk::DescriptorSet > vds = static_cast< vk::Device >( *_device)
      .allocateDescriptorSets( dci );

    assert( !vds.empty( ) );

    _descriptorSet = vds.front( );
  }

  DescriptorSet::~DescriptorSet( void )
  {
    static_cast< vk::Device > ( *_device ).freeDescriptorSets( *_descriptorPool, 
      _descriptorSet );
  }
  DescriptorImageInfo::DescriptorImageInfo( vk::ImageLayout imageLayout_, 
    const std::shared_ptr<ImageView>& imageView_, 
    const std::shared_ptr<Sampler>& sampler_ )
    : imageLayout( imageLayout_ )
    , imageView( imageView_ )
    , sampler( sampler_ )
  {
  }
  DescriptorImageInfo::DescriptorImageInfo( const DescriptorImageInfo& rhs )
    : DescriptorImageInfo( rhs.imageLayout, rhs.imageView, rhs.sampler )
  {
  }
  DescriptorImageInfo & DescriptorImageInfo::operator=( const DescriptorImageInfo& rhs )
  {
    imageLayout = rhs.imageLayout;
    imageView = rhs.imageView;
    sampler = rhs.sampler;
    return *this;
  }
  WriteDescriptorSet::WriteDescriptorSet( const std::shared_ptr<DescriptorSet>& dstSet_, 
    uint32_t dstBinding_, uint32_t dstArrayElement_, vk::DescriptorType descriptorType_, 
    uint32_t descriptorCount_, 
    vk::Optional<const DescriptorImageInfo> imageInfo_, 
    vk::Optional<const DescriptorBufferInfo> bufferInfo_, 
    const std::shared_ptr<lava::BufferView>& bufferView_ )
    : dstSet( dstSet_ )
    , dstBinding( dstBinding_ )
    , dstArrayElement( dstArrayElement_ )
    , descriptorType( descriptorType_ )
    , descriptorCount( descriptorCount_ )
    , imageInfo( imageInfo_ ? new DescriptorImageInfo( *imageInfo_ ) : nullptr )
    , bufferInfo( bufferInfo_ ? new DescriptorBufferInfo( *bufferInfo_ ) : nullptr )
    , texelBufferView( bufferView_ )
  {
  }
  WriteDescriptorSet::WriteDescriptorSet( const WriteDescriptorSet & rhs )
    : WriteDescriptorSet( rhs.dstSet, rhs.dstBinding, rhs.dstArrayElement, 
      rhs.descriptorType, rhs.descriptorCount,
      rhs.imageInfo.get( ) ? 
        vk::Optional<const DescriptorImageInfo>( *rhs.imageInfo )
          : vk::Optional<const DescriptorImageInfo>( nullptr ),
      rhs.bufferInfo.get( ) ? 
        vk::Optional<const DescriptorBufferInfo>( *rhs.bufferInfo ) 
          : vk::Optional<const DescriptorBufferInfo>( nullptr ),
      rhs.texelBufferView )
  {
  }
  WriteDescriptorSet & WriteDescriptorSet::operator=( const WriteDescriptorSet & rhs )
  {
    dstSet = rhs.dstSet;
    dstBinding = rhs.dstBinding;
    dstArrayElement = rhs.dstArrayElement;
    descriptorType = rhs.descriptorType;
    descriptorCount = rhs.descriptorCount;
    imageInfo.reset( rhs.imageInfo ? 
      new DescriptorImageInfo( *rhs.imageInfo ) : nullptr );
    bufferInfo.reset( rhs.bufferInfo ? 
      new DescriptorBufferInfo( *rhs.bufferInfo ) : nullptr );
    texelBufferView = rhs.texelBufferView;
    return *this;
  }
  CopyDescriptorSet::CopyDescriptorSet( 
    const std::shared_ptr<DescriptorSet>& srcSet_, uint32_t srcBinding_,
    uint32_t srcArrayElem_, const std::shared_ptr<DescriptorSet>& dstSet_,
    uint32_t dstBinding_, uint32_t dstArrayElem_, uint32_t descriptorCount_ )
    : srcSet( srcSet_ )
    , srcBinding( srcBinding_ )
    , srcArrayElement( srcArrayElem_ )
    , dstSet( dstSet_ )
    , dstBinding( dstBinding_ )
    , dstArrayElement( dstArrayElem_ )
    , descriptorCount( descriptorCount_ )
  {
  }
  CopyDescriptorSet::CopyDescriptorSet( const CopyDescriptorSet & cds )
    : CopyDescriptorSet( cds.srcSet, cds.srcBinding, cds.srcArrayElement, 
      cds.dstSet, cds.dstBinding, cds.dstArrayElement, cds.descriptorCount )
  {
  }
  CopyDescriptorSet& CopyDescriptorSet::operator=( const CopyDescriptorSet & cds )
  {
    srcSet = cds.srcSet;
    srcBinding = cds.srcBinding;
    srcArrayElement = cds.srcArrayElement;
    dstSet = cds.dstSet;
    dstBinding = cds.dstBinding;
    dstArrayElement = cds.dstArrayElement;
    descriptorCount = cds.descriptorCount;

    return *this;
  }
}