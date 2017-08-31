#include "Pipeline.h"

#include "Device.h"
#include "VulkanResource.h"

#include <fstream>

namespace lava
{
  ShaderModule::ShaderModule( const DeviceRef& device,
    const std::string & filePath )
    : ShaderModule( device, readFile( filePath ) )
  {
  }

  const std::vector<uint32_t> ShaderModule::readFile( const std::string & filename )
  {
    std::ifstream file( filename, std::ios::ate | std::ios::binary );

    if ( !file.is_open( ) )
    {
      throw std::runtime_error( "failed to open file!" );
    }

    size_t fileSize = ( size_t ) file.tellg( );
    std::vector<char> buffer( fileSize );

    file.seekg( 0 );
    file.read( buffer.data( ), fileSize );

    file.close( );

    const uint32_t* arr = reinterpret_cast< const uint32_t* >( buffer.data( ) );

    return std::vector<uint32_t>( arr, arr + buffer.size( ) );
  }

  ShaderModule::ShaderModule( const DeviceRef& device,
    vk::ArrayProxy<const uint32_t> code )
    : VulkanResource( device )
  {
    vk::ShaderModuleCreateInfo sci(
      vk::ShaderModuleCreateFlags( ),
      4 * code.size( ),
      code.data( )
    );
    _shaderModule = static_cast< vk::Device >( *_device ).createShaderModule( sci );
  }

  ShaderModule::~ShaderModule( )
  {
    static_cast< vk::Device >( *_device ).destroyShaderModule( _shaderModule );
  }

  PipelineVertexInputStateCreateInfo::PipelineVertexInputStateCreateInfo(
    vk::ArrayProxy<const vk::VertexInputBindingDescription> vertexBindingDescriptions_,
    vk::ArrayProxy<const vk::VertexInputAttributeDescription> vertexAttributeDesriptions_ )
    : vertexBindingDescriptions( vertexBindingDescriptions_.begin( ), vertexBindingDescriptions_.end( ) )
    , vertexAttributeDesriptions( vertexAttributeDesriptions_.begin( ), vertexAttributeDesriptions_.end( ) )
  {}

  PipelineVertexInputStateCreateInfo::PipelineVertexInputStateCreateInfo(
    const PipelineVertexInputStateCreateInfo& rhs )
    : PipelineVertexInputStateCreateInfo( rhs.vertexBindingDescriptions, rhs.vertexAttributeDesriptions )
  {}

  PipelineVertexInputStateCreateInfo & PipelineVertexInputStateCreateInfo::operator=(
    PipelineVertexInputStateCreateInfo const& rhs )
  {
    vertexBindingDescriptions = rhs.vertexBindingDescriptions;
    vertexAttributeDesriptions = rhs.vertexAttributeDesriptions;
    return *this;
  }

  PipelineDynamicStateCreateInfo::PipelineDynamicStateCreateInfo(
    vk::ArrayProxy<const vk::DynamicState> dynamicStates_ )
    : dynamicStates( dynamicStates_.begin( ), dynamicStates_.end( ) )
  {}

  PipelineDynamicStateCreateInfo::PipelineDynamicStateCreateInfo(
    const PipelineDynamicStateCreateInfo& rhs )
    : PipelineDynamicStateCreateInfo( rhs.dynamicStates )
  {}

  PipelineDynamicStateCreateInfo & PipelineDynamicStateCreateInfo::operator=(
    const PipelineDynamicStateCreateInfo& rhs )
  {
    dynamicStates = rhs.dynamicStates;
    return *this;
  }



  PipelineViewportStateCreateInfo::PipelineViewportStateCreateInfo(
    vk::ArrayProxy<const vk::Viewport> viewports_, vk::ArrayProxy<const vk::Rect2D> scissors_ )
    : viewports( viewports_.begin( ), viewports_.end( ) )
    , scissors( scissors_.begin( ), scissors_.end( ) )
  {}

  PipelineViewportStateCreateInfo::PipelineViewportStateCreateInfo(
    const PipelineViewportStateCreateInfo& rhs )
    : PipelineViewportStateCreateInfo( rhs.viewports, rhs.scissors )
  {}

  PipelineViewportStateCreateInfo & PipelineViewportStateCreateInfo::operator=(
    const PipelineViewportStateCreateInfo& rhs )
  {
    viewports = rhs.viewports;
    scissors = rhs.scissors;
    return *this;
  }

  PipelineCache::PipelineCache( const DeviceRef& device, vk::PipelineCacheCreateFlags flags,
    size_t initialSize, void const* initialData )
    : VulkanResource( device )
  {
    vk::PipelineCacheCreateInfo createInfo{ flags, initialSize, initialData };
    _pipelineCache = static_cast< vk::Device >( *_device ).createPipelineCache( createInfo );
  }

  PipelineCache::~PipelineCache( )
  {
    static_cast< vk::Device >( *_device ).destroyPipelineCache( _pipelineCache );
  }

  std::vector<uint8_t>  PipelineCache::getData( ) const
  {
    return static_cast< vk::Device >( *_device ).getPipelineCacheData( _pipelineCache );
  }

  void PipelineCache::merge( vk::ArrayProxy<const std::shared_ptr<PipelineCache>> srcCaches ) const
  {
    std::vector<vk::PipelineCache> caches;
    caches.reserve( srcCaches.size( ) );
    for ( auto const& c : srcCaches )
    {
      caches.push_back( *c );
    }
    static_cast< vk::Device >( *_device ).mergePipelineCaches( _pipelineCache, caches );
  }
  Pipeline::Pipeline( const DeviceRef& device )
    : VulkanResource( device )
  {}

  void Pipeline::setPipeline( vk::Pipeline const& pipeline )
  {
    _pipeline = pipeline;
  }

  Pipeline::~Pipeline( )
  {
    static_cast< vk::Device >( *_device ).destroyPipeline( _pipeline );
  }


  ComputePipeline::ComputePipeline( const DeviceRef& device,
    const std::shared_ptr<PipelineCache>& pipelineCache,
    vk::PipelineCreateFlags flags,
    const PipelineShaderStageCreateInfo& stage,
    const std::shared_ptr<PipelineLayout>& layout,
    const std::shared_ptr<Pipeline>& basePipelineHandle,
    int32_t basePipelineIndex )
    : Pipeline( device )
  {
    vk::SpecializationInfo vkSpecializationInfo(
      stage.specializationInfo->mapEntries.size( ),
      stage.specializationInfo->mapEntries.data( ),
      stage.specializationInfo->data.size( ),
      stage.specializationInfo->data.data( ) );
    vk::PipelineShaderStageCreateInfo vkStage(
    {},
      stage.stage,
      stage.module ? static_cast< vk::ShaderModule >( *stage.module ) : nullptr,
      stage.name.data( ),
      &vkSpecializationInfo
    );

    vk::ComputePipelineCreateInfo cci(
      flags,
      vkStage,
      layout ? static_cast< vk::PipelineLayout >( *layout ) : nullptr,
      basePipelineHandle ? static_cast< vk::Pipeline >( *basePipelineHandle ) : nullptr,
      basePipelineIndex
    );

    setPipeline( vk::Device( *_device ).createComputePipeline(
      pipelineCache ? static_cast< vk::PipelineCache >( *pipelineCache ) : nullptr,
      cci ) );
  }

  GraphicsPipeline::GraphicsPipeline( const DeviceRef& device,
    const std::shared_ptr<PipelineCache>& pipelineCache,
    vk::Optional<const PipelineVertexInputStateCreateInfo> vertexInputState,
    vk::Optional<const vk::PipelineInputAssemblyStateCreateInfo> inputAssemblyState,
    vk::Optional<const PipelineViewportStateCreateInfo> viewportState,
    vk::Optional<const PipelineDynamicStateCreateInfo> dynamicState,
    vk::Optional<const vk::PipelineRasterizationStateCreateInfo> rasterizationState,
    vk::Optional<const vk::PipelineMultisampleStateCreateInfo> multisampleState,
    vk::Optional<const vk::PipelineDepthStencilStateCreateInfo> depthStencilState )
    : Pipeline( device )
  {
    vk::PipelineVertexInputStateCreateInfo vkVertexInputState;
    if ( vertexInputState )
    {
      vkVertexInputState = vk::PipelineVertexInputStateCreateInfo(
      {},
        vertexInputState->vertexBindingDescriptions.size( ),
        vertexInputState->vertexBindingDescriptions.data( ),
        vertexInputState->vertexAttributeDesriptions.size( ),
        vertexInputState->vertexAttributeDesriptions.data( )
      );
    }

    vk::PipelineViewportStateCreateInfo vkViewportState;
    if ( viewportState )
    {
      vkViewportState = vk::PipelineViewportStateCreateInfo(
      {},
        viewportState->viewports.size( ),
        viewportState->viewports.data( ),
        viewportState->scissors.size( ),
        viewportState->scissors.data( )
      );
    }

    vk::PipelineDynamicStateCreateInfo vkDynamicState;
    if ( dynamicState )
    {
      vkDynamicState = vk::PipelineDynamicStateCreateInfo(
      {},
        dynamicState->dynamicStates.size( ),
        dynamicState->dynamicStates.data( )
      );
    }

    vk::GraphicsPipelineCreateInfo gci;
    gci.setPVertexInputState( vertexInputState ? &vkVertexInputState : nullptr );
    gci.setPTessellationState( nullptr );
    gci.setPInputAssemblyState( inputAssemblyState );
    gci.setPDynamicState( dynamicState ? &vkDynamicState : nullptr );
    gci.setPViewportState( viewportState ? &vkViewportState : nullptr );
    gci.setPMultisampleState( multisampleState );
    gci.setPDepthStencilState( depthStencilState );

    setPipeline( vk::Device( *_device ).createGraphicsPipeline(
      pipelineCache ? static_cast< vk::PipelineCache >( *pipelineCache ) : nullptr,
      gci ) );
  }


  PipelineLayout::PipelineLayout( const std::shared_ptr<Device>& device, vk::ArrayProxy<const std::shared_ptr<DescriptorSetLayout>> setLayouts,
    vk::ArrayProxy<const vk::PushConstantRange> pushConstantRanges )
    : VulkanResource( device )
    , _setLayouts( setLayouts.begin( ), setLayouts.end( ) )
  {
    std::vector<vk::DescriptorSetLayout> dsl;
    dsl.reserve( setLayouts.size( ) );
    for ( auto const& l : setLayouts )
    {
      dsl.push_back( static_cast< vk::DescriptorSetLayout >( *l ) );
    }

    vk::PipelineLayoutCreateInfo pci( {}, dsl.size( ), dsl.data( ), pushConstantRanges.size( ), pushConstantRanges.data( ) );
    _pipelineLayout = static_cast< vk::Device >( *_device ).createPipelineLayout( pci );
  }

  PipelineLayout::~PipelineLayout( )
  {
    static_cast< vk::Device >( *_device ).destroyPipelineLayout( _pipelineLayout );
  }








  SpecializationInfo::SpecializationInfo( vk::ArrayProxy<const vk::SpecializationMapEntry> mapEntries_, vk::ArrayProxy<const uint8_t> data_ )
    : mapEntries( mapEntries_.begin( ), mapEntries_.end( ) )
    , data( data_.begin( ), data_.end( ) )
  {}

  SpecializationInfo::SpecializationInfo( SpecializationInfo const& rhs )
    : SpecializationInfo( rhs.mapEntries, rhs.data )
  {}

  SpecializationInfo & SpecializationInfo::operator=( SpecializationInfo const& rhs )
  {
    mapEntries = rhs.mapEntries;
    return *this;
  }

  PipelineShaderStageCreateInfo::PipelineShaderStageCreateInfo( vk::ShaderStageFlagBits stage_, std::shared_ptr<ShaderModule> const& module_, std::string const& name_,
    vk::Optional<const SpecializationInfo> specializationInfo_ )
    : stage( stage_ )
    , module( module_ )
    , name( name_ )
    , specializationInfo( specializationInfo_ ? new SpecializationInfo( *specializationInfo_ ) : nullptr )
  {}

  PipelineShaderStageCreateInfo::PipelineShaderStageCreateInfo( PipelineShaderStageCreateInfo const& rhs )
    : PipelineShaderStageCreateInfo( rhs.stage, rhs.module, rhs.name, rhs.specializationInfo ? vk::Optional<const SpecializationInfo>( *rhs.specializationInfo.get( ) ) : nullptr )
  {}

  PipelineShaderStageCreateInfo & PipelineShaderStageCreateInfo::operator=( PipelineShaderStageCreateInfo const& rhs )
  {
    stage = rhs.stage;
    module = rhs.module;
    name = rhs.name;
    specializationInfo.reset( rhs.specializationInfo ? new SpecializationInfo( *rhs.specializationInfo ) : nullptr );
    return *this;
  }
}