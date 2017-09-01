#include "Pipeline.h"

#include "Device.h"
#include "VulkanResource.h"
#include "RenderPass.h"

#include <fstream>

namespace lava
{
  ShaderModule::ShaderModule( const DeviceRef& device, 
  const std::string& filePath, vk::ShaderStageFlagBits /* todo: UNUSE type */ )
    : VulkanResource( device )
  {
    std::ifstream file( filePath, std::ios::ate | std::ios::binary );

    if ( !file.is_open( ) ) {
      std::cerr << "File don't opened" << std::endl;
      throw std::runtime_error( "failed to open file!" );
    }

    size_t fileSize = ( size_t ) file.tellg( );
    std::vector<char> code( fileSize );

    file.seekg( 0 );
    file.read( code.data( ), fileSize );

    file.close( );

    vk::ShaderModuleCreateInfo sci( 
      vk::ShaderModuleCreateFlags(), 
      code.size(), 
      reinterpret_cast<const uint32_t*>( code.data( ) )
    );

    _shaderModule = static_cast< vk::Device > ( *_device ).createShaderModule( sci );
  }
  ShaderModule::ShaderModule( const DeviceRef& device,
    const std::string& filePath )
    : ShaderModule( device, readFile( filePath ) )
  {
  }

  const std::vector<uint32_t> ShaderModule::readFile( const std::string& filename )
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


  PipelineColorBlendStateCreateInfo::PipelineColorBlendStateCreateInfo( 
    bool logicEnable_, vk::LogicOp logicOp_, 
    vk::ArrayProxy<const vk::PipelineColorBlendAttachmentState> attachments_, 
    std::array<float, 4> const& blendConstants_ )
    : logicEnable( logicEnable_ )
    , logicOp( logicOp_ )
    , attachments( attachments_.begin( ), attachments_.end( ) )
    , blendConstants( blendConstants_ )
  {}

  PipelineColorBlendStateCreateInfo::PipelineColorBlendStateCreateInfo( 
    PipelineColorBlendStateCreateInfo const& rhs )
    : PipelineColorBlendStateCreateInfo( rhs.logicEnable, rhs.logicOp, 
      rhs.attachments, rhs.blendConstants )
  {}

  PipelineColorBlendStateCreateInfo& 
    PipelineColorBlendStateCreateInfo::operator=( 
      PipelineColorBlendStateCreateInfo const& rhs )
  {
    logicEnable = rhs.logicEnable;
    logicOp = rhs.logicOp;
    attachments = rhs.attachments;
    blendConstants = rhs.blendConstants;
    return *this;
  }


  PipelineMultisampleStateCreateInfo::PipelineMultisampleStateCreateInfo( 
    vk::SampleCountFlagBits rasterizationSamples_, bool sampleShadingEnable_, 
    float minSampleShading_, vk::ArrayProxy<const vk::SampleMask> sampleMasks_, 
    bool alphaToCoverageEnable_, bool alphaToOneEnable_ )
    : rasterizationSamples( rasterizationSamples_ )
    , sampleShadingEnable( sampleShadingEnable_ )
    , minSampleShading( minSampleShading_ )
    , sampleMasks( sampleMasks_.begin( ), sampleMasks_.end( ) )
    , alphaToCoverageEnable( alphaToCoverageEnable_ )
    , alphaToOneEnable( alphaToOneEnable_ )
  {
    assert( sampleMasks.empty( ) || 
      ( ceil( static_cast<uint32_t>( sampleShadingEnable ) / 32 ) <= sampleMasks.size( ) ) );
  }

  PipelineMultisampleStateCreateInfo::PipelineMultisampleStateCreateInfo( 
    PipelineMultisampleStateCreateInfo const& rhs )
    : PipelineMultisampleStateCreateInfo( rhs.rasterizationSamples, 
      rhs.sampleShadingEnable, rhs.minSampleShading, rhs.sampleMasks, 
      rhs.alphaToCoverageEnable, rhs.alphaToOneEnable )
  {}

  PipelineMultisampleStateCreateInfo& 
    PipelineMultisampleStateCreateInfo::operator=( 
      PipelineMultisampleStateCreateInfo const& rhs )
  {
    rasterizationSamples = rhs.rasterizationSamples;
    sampleShadingEnable = rhs.sampleShadingEnable;
    minSampleShading = rhs.minSampleShading;
    sampleMasks = rhs.sampleMasks;
    alphaToCoverageEnable = rhs.alphaToCoverageEnable;
    alphaToOneEnable = rhs.alphaToOneEnable;
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
    uint32_t basePipelineIndex )
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

  GraphicsPipeline::GraphicsPipeline( const std::shared_ptr<Device>& device, 
    const std::shared_ptr<PipelineCache>& pipelineCache, 
    vk::PipelineCreateFlags flags, 
    vk::ArrayProxy<const PipelineShaderStageCreateInfo> stages, 
    vk::Optional<const PipelineVertexInputStateCreateInfo> vertexInputState,
    vk::Optional<const vk::PipelineInputAssemblyStateCreateInfo> inputAssemblyState, 
    vk::Optional<const vk::PipelineTessellationStateCreateInfo> tessellationState,
    vk::Optional<const PipelineViewportStateCreateInfo> viewportState, 
    vk::Optional<const vk::PipelineRasterizationStateCreateInfo> rasterizationState,
    vk::Optional<const PipelineMultisampleStateCreateInfo> multisampleState, 
    vk::Optional<const vk::PipelineDepthStencilStateCreateInfo> depthStencilState,
    vk::Optional<const PipelineColorBlendStateCreateInfo> colorBlendState, 
    vk::Optional<const PipelineDynamicStateCreateInfo> dynamicState,
    std::shared_ptr<PipelineLayout> const& pipelineLayout, 
    std::shared_ptr<RenderPass> const& renderPass, uint32_t subpass,
    std::shared_ptr<Pipeline> const& basePipelineHandle, uint32_t basePipelineIdx )
    : Pipeline( device )
  {
    std::vector<vk::SpecializationInfo> specializationInfos;
    specializationInfos.reserve( stages.size( ) );

    std::vector<vk::PipelineShaderStageCreateInfo> vkStages;
    vkStages.reserve( stages.size( ) );
    for ( auto const& s : stages )
    {
      if ( s.specializationInfo )
      {
        specializationInfos.push_back( vk::SpecializationInfo( s.specializationInfo->mapEntries.size( ), s.specializationInfo->mapEntries.data( ),
          s.specializationInfo->data.size( ), s.specializationInfo->data.data( ) ) );
      }
      vkStages.push_back( vk::PipelineShaderStageCreateInfo( {}, s.stage, s.module ? static_cast<vk::ShaderModule>( *s.module ) : nullptr, s.name.data( ),
        s.specializationInfo ? &specializationInfos.back( ) : nullptr ) );
    }

    vk::PipelineVertexInputStateCreateInfo vkVertexInputState;
    if ( vertexInputState )
    {
      vkVertexInputState = vk::PipelineVertexInputStateCreateInfo( {}, vertexInputState->vertexBindingDescriptions.size( ), vertexInputState->vertexBindingDescriptions.data( ),
        vertexInputState->vertexAttributeDesriptions.size( ), vertexInputState->vertexAttributeDesriptions.data( ) );
    }

    vk::PipelineViewportStateCreateInfo vkViewportState;
    if ( viewportState )
    {
      vkViewportState = vk::PipelineViewportStateCreateInfo( {}, viewportState->viewports.size( ), viewportState->viewports.data( ),
        viewportState->scissors.size( ), viewportState->scissors.data( ) );
    }

    vk::PipelineMultisampleStateCreateInfo vkMultisampleState;
    if ( multisampleState )
    {
      vkMultisampleState = vk::PipelineMultisampleStateCreateInfo( {}, multisampleState->rasterizationSamples, multisampleState->sampleShadingEnable, multisampleState->minSampleShading,
        multisampleState->sampleMasks.empty( ) ? nullptr : multisampleState->sampleMasks.data( ), multisampleState->alphaToCoverageEnable,
        multisampleState->alphaToOneEnable );
    }

    vk::PipelineColorBlendStateCreateInfo vkColorBlendState;
    if ( colorBlendState )
    {
      vkColorBlendState = vk::PipelineColorBlendStateCreateInfo( {}, colorBlendState->logicEnable, colorBlendState->logicOp,  colorBlendState->attachments.size( ),
        colorBlendState->attachments.data( ), colorBlendState->blendConstants );
    }

    vk::PipelineDynamicStateCreateInfo vkDynamicState;
    if ( dynamicState )
    {
      vkDynamicState = vk::PipelineDynamicStateCreateInfo( {}, dynamicState->dynamicStates.size( ), dynamicState->dynamicStates.data( ) );
    }

    /*vk::GraphicsPipelineCreateInfo pci(
      flags,
      vkStages.size( ),
      vkStages.data( ),
      vertexInputState ? &vkVertexInputState : nullptr,
      inputAssemblyState,
      tessellationState,
      viewportState ? &vkViewportState : nullptr,
      rasterizationState, multisampleState ? &vkMultisampleState : nullptr,
      depthStencilState,
      colorBlendState ? &vkColorBlendState : nullptr,
      dynamicState ? &vkDynamicState : nullptr,
      pipelineLayout ? *pipelineLayout : vk::PipelineLayout( ),
      renderPass ? *renderPass : vk::RenderPass( ),
      subpass, basePipelineHandle ? *basePipelineHandle : vk::Pipeline( ),
      basePipelineIdx );*/

    vk::GraphicsPipelineCreateInfo pci;
    pci.stageCount = vkStages.size( );
    pci.pStages = vkStages.data( );
    pci.pVertexInputState = vertexInputState ? &vkVertexInputState : nullptr;
    pci.pInputAssemblyState = inputAssemblyState;
    pci.pViewportState = viewportState ? &vkViewportState : nullptr;
    pci.pRasterizationState = rasterizationState;
    pci.pMultisampleState = multisampleState ? &vkMultisampleState : nullptr;
    pci.pDepthStencilState = depthStencilState;
    pci.pColorBlendState = colorBlendState ? &vkColorBlendState : nullptr;
    pci.pDynamicState = dynamicState ? &vkDynamicState : nullptr;
    pci.layout = pipelineLayout ? *pipelineLayout : vk::PipelineLayout( );
    pci.renderPass = renderPass ? *renderPass : vk::RenderPass( );
    pci.subpass = subpass;
    pci.basePipelineHandle = basePipelineHandle ? *basePipelineHandle : vk::Pipeline( );
    pci.basePipelineIndex = basePipelineIdx;

    setPipeline( static_cast<vk::Device>( *_device ).createGraphicsPipeline( pipelineCache ? *pipelineCache : vk::PipelineCache( ), pci ) );
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