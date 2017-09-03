#include "Descriptor.h"

#include "Device.h"

namespace lava
{
  DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(
    uint32_t binding_, vk::DescriptorType descriptorType_,
    vk::ShaderStageFlags stageFlags_ )
    : binding( binding_ )
    , descriptorType( descriptorType_ )
    , stageFlags( stageFlags_ )
    , immutableSamplers( { } )
  {}

  DescriptorSetLayoutBinding::DescriptorSetLayoutBinding( 
    DescriptorSetLayoutBinding const& rhs )
    : DescriptorSetLayoutBinding( rhs.binding, rhs.descriptorType, rhs.stageFlags )
  {}

  DescriptorSetLayoutBinding& 
  DescriptorSetLayoutBinding::operator=( DescriptorSetLayoutBinding const& rhs )
  {
    binding = rhs.binding;
    descriptorType = rhs.descriptorType;
    stageFlags = rhs.stageFlags;
    return *this;
  }


  DescriptorBufferInfo::DescriptorBufferInfo( const std::shared_ptr<Buffer>& buffer_, 
    vk::DeviceSize offset_, vk::DeviceSize range_ )
    : buffer( buffer_ )
    , offset( offset_ )
    , range( range_ )
  {

  }
  DescriptorBufferInfo::DescriptorBufferInfo( const DescriptorBufferInfo& rhs )
    : DescriptorBufferInfo( rhs.buffer, rhs.offset, rhs.range )
  {
  }
  DescriptorBufferInfo& DescriptorBufferInfo::operator=( const DescriptorBufferInfo& rhs )
  {
    buffer = rhs.buffer;
    offset = rhs.offset;
    range = rhs.range;
    return *this;
  }



  DescriptorSetLayout::DescriptorSetLayout( const DeviceRef& device,
    vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings )
    : VulkanResource( device )
    , _bindings( bindings.begin( ), bindings.end( ) )
  {
    std::vector<vk::DescriptorSetLayoutBinding> dslb;
    dslb.reserve( _bindings.size( ) );

    for ( auto b : _bindings )
    {
      dslb.push_back( vk::DescriptorSetLayoutBinding( b.binding, b.descriptorType, 1, b.stageFlags, nullptr ) );
    }
    vk::DescriptorSetLayoutCreateInfo dci(
      {},
      dslb.size( ),
      dslb.data( )
    );
    _descriptorSetLayout = static_cast< vk::Device >( *_device )
      .createDescriptorSetLayout( dci );
  }

  DescriptorSetLayout::~DescriptorSetLayout( void )
  {
    static_cast< vk::Device >( *_device ).destroyDescriptorSetLayout( _descriptorSetLayout );
  }


  DescriptorPool::DescriptorPool( const DeviceRef& device,
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

    _descriptorPool = static_cast< vk::Device >( *_device ).createDescriptorPool( dci );
  }

  DescriptorPool::~DescriptorPool( void )
  {
    static_cast< vk::Device >( *_device ).destroyDescriptorPool( _descriptorPool );
  }

  void DescriptorPool::reset( void )
  {
    static_cast< vk::Device >( *_device ).resetDescriptorPool( _descriptorPool );
  }

  DescriptorSet::DescriptorSet( const DeviceRef& device, 
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
  /*DescriptorImageInfo::DescriptorImageInfo( vk::ImageLayout imageLayout_, 
    const std::shared_ptr<ImageView>& imageView_, const std::shared_ptr<Sampler>& sampler_ )
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
  }*/
  WriteDescriptorSet::WriteDescriptorSet( const std::shared_ptr<DescriptorSet>& dstSet_, 
    uint32_t dstBinding_, uint32_t dstArrayElement_, vk::DescriptorType descriptorType_, 
    uint32_t descriptorCount_, 
    vk::Optional<const DescriptorImageInfo> imageInfo_, 
    vk::Optional<const DescriptorBufferInfo> bufferInfo_ )
    : dstSet( dstSet_ )
    , dstBinding( dstBinding_ )
    , dstArrayElement( dstArrayElement_ )
    , descriptorType( descriptorType_ )
    , descriptorCount( descriptorCount_ )
    , imageInfo( imageInfo_ ? new DescriptorImageInfo( *imageInfo_ ) : nullptr )
    , bufferInfo( bufferInfo_ ? new DescriptorBufferInfo( *bufferInfo_ ) : nullptr )
  {
  }
  WriteDescriptorSet::WriteDescriptorSet( const WriteDescriptorSet & rhs )
    : WriteDescriptorSet( rhs.dstSet, rhs.dstBinding, rhs.dstArrayElement, 
      rhs.descriptorType, rhs.descriptorCount,
      rhs.imageInfo.get( ) ? vk::Optional<const DescriptorImageInfo>( *rhs.imageInfo )
        : vk::Optional<const DescriptorImageInfo>( nullptr ),
      rhs.bufferInfo.get( ) ? vk::Optional<const DescriptorBufferInfo>( *rhs.bufferInfo ) 
      : vk::Optional<const DescriptorBufferInfo>( nullptr ) )
  {
  }
  WriteDescriptorSet & WriteDescriptorSet::operator=( const WriteDescriptorSet & rhs )
  {
    dstSet = rhs.dstSet;
    dstBinding = rhs.dstBinding;
    dstArrayElement = rhs.dstArrayElement;
    descriptorType = rhs.descriptorType;
    descriptorCount = rhs.descriptorCount;
    imageInfo.reset( rhs.imageInfo ? new DescriptorImageInfo( *rhs.imageInfo ) : nullptr );
    bufferInfo.reset( rhs.bufferInfo ? new DescriptorBufferInfo( *rhs.bufferInfo ) : nullptr );
    return *this;
  }
}