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

    if ( !file.is_open( ) )
    {
      std::cerr << "File " << filePath << " don't opened" << std::endl;
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
    vk::ArrayProxy<const vk::VertexInputBindingDescription> vBindingDescriptions_,
    vk::ArrayProxy<const vk::VertexInputAttributeDescription> vAttrirDescriptions_ )
    : vertexBindingDescriptions( 
      vBindingDescriptions_.begin( ), vBindingDescriptions_.end( )
    )
    , vertexAttrirDescriptions( 
      vAttrirDescriptions_.begin( ), vAttrirDescriptions_.end( )
    )
  {
  }

  PipelineVertexInputStateCreateInfo::PipelineVertexInputStateCreateInfo(
    const PipelineVertexInputStateCreateInfo& rhs )
    : PipelineVertexInputStateCreateInfo( rhs.vertexBindingDescriptions, rhs.vertexAttrirDescriptions )
  {
  }

  PipelineVertexInputStateCreateInfo & PipelineVertexInputStateCreateInfo::operator=(
    PipelineVertexInputStateCreateInfo const& rhs )
  {
    vertexBindingDescriptions = rhs.vertexBindingDescriptions;
    vertexAttrirDescriptions = rhs.vertexAttrirDescriptions;
    return *this;
  }

  PipelineDynamicStateCreateInfo::PipelineDynamicStateCreateInfo(
    vk::ArrayProxy<const vk::DynamicState> dynamicStates_ )
    : dynamicStates( dynamicStates_.begin( ), dynamicStates_.end( ) )
  {
  }

  PipelineDynamicStateCreateInfo::PipelineDynamicStateCreateInfo(
    const PipelineDynamicStateCreateInfo& rhs )
    : PipelineDynamicStateCreateInfo( rhs.dynamicStates )
  {
  }

  PipelineDynamicStateCreateInfo & PipelineDynamicStateCreateInfo::operator=(
    const PipelineDynamicStateCreateInfo& rhs )
  {
    dynamicStates = rhs.dynamicStates;
    return *this;
  }


  PipelineViewportStateCreateInfo::PipelineViewportStateCreateInfo
    ( uint32_t dummyViews, uint32_t dummySci )
    : PipelineViewportStateCreateInfo( 
      std::vector< vk::Viewport>( dummyViews ), 
      std::vector< vk::Rect2D>( dummySci )
    )
  {
  }

  PipelineViewportStateCreateInfo::PipelineViewportStateCreateInfo(
    vk::ArrayProxy<const vk::Viewport> viewports_, 
    vk::ArrayProxy<const vk::Rect2D> scissors_ )
    : viewports( viewports_.begin( ), viewports_.end( ) )
    , scissors( scissors_.begin( ), scissors_.end( ) )
  {
  }

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
  {
  }

  PipelineColorBlendStateCreateInfo::PipelineColorBlendStateCreateInfo( 
    PipelineColorBlendStateCreateInfo const& rhs )
    : PipelineColorBlendStateCreateInfo( rhs.logicEnable, rhs.logicOp, 
      rhs.attachments, rhs.blendConstants )
  {
  }

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
  {
  }

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

  PipelineCache::PipelineCache( const DeviceRef& device, const char* filename )
    : VulkanResource( device )
  {
    // Check disk for existing cache data
    size_t startCacheSize = 0;
    void *startCacheData = nullptr;

    FILE *pReadFile = fopen(filename, "rb");
    if (pReadFile)
    {
      // Determine cache size
      fseek(pReadFile, 0, SEEK_END);
      startCacheSize = ftell(pReadFile);
      rewind(pReadFile);

      // Allocate memory to hold the initial cache data
      startCacheData = (char *)malloc(sizeof(char) * startCacheSize);
      if (startCacheData == nullptr)
      {
        fputs("Memory error", stderr);
        throw;
      }

      // Read the data into our buffer
      size_t result = fread(startCacheData, 1, startCacheSize, pReadFile);
      if (result != startCacheSize)
      {
        fputs("Reading error", stderr);
        free(startCacheData);
        throw;
      }

      // Clean up and print results
      fclose(pReadFile);
      printf( "  Pipeline cache HIT!\n" );
      printf("  cacheData loaded from %s\n", filename );

    } else {
      // No cache found on disk
      printf("  Pipeline cache miss!\n");
    }

    // https://github.com/LunarG/VulkanSamples/blob/master/API-Samples/pipeline_cache/pipeline_cache.cpp 
    // TODO: NOT COMPLETE 

    vk::PipelineCacheCreateInfo createInfo{ {}, startCacheSize, startCacheData };
    _pipelineCache = static_cast< vk::Device >( *_device ).createPipelineCache( createInfo );
  }

  PipelineCache::PipelineCache( const DeviceRef& device, vk::PipelineCacheCreateFlags flags,
    size_t initialSize, void const* initialData )
    : VulkanResource( device )
  {
    vk::PipelineCacheCreateInfo createInfo{ flags, initialSize, initialData };
    _pipelineCache = static_cast< vk::Device >( *_device ).createPipelineCache( createInfo );
  }

  PipelineCache::~PipelineCache( void )
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

  void PipelineCache::saveToFile( const std::string& filename )
  {
    saveToFile( filename.c_str( ) );
  }

  void PipelineCache::saveToFile( const char* filename )
  {
    size_t size = 0;
    vk::Result result = static_cast<vk::Device>(*_device)
      .getPipelineCacheData( _pipelineCache, &size, nullptr );
    if ( result == vk::Result::eSuccess && size != 0 )
    {
      auto myfile = std::fstream( filename, std::ios::out | std::ios::binary );
      void* data = ( char * ) malloc( sizeof( char ) * size );
      result = static_cast<vk::Device>(*_device)
        .getPipelineCacheData( _pipelineCache, &size, data );
      if ( result == vk::Result::eSuccess )
      {
        myfile.write( (char*)data, size );
        myfile.close( );
      }
      free( data );
    }
  }

  Pipeline::Pipeline( const DeviceRef& device )
    : VulkanResource( device )
  {
  }

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
    vk::PipelineCreateFlags flags, const PipelineShaderStageCreateInfo& stage,
    const std::shared_ptr<PipelineLayout>& layout,
    const std::shared_ptr<Pipeline>& basePipelineHandle, uint32_t basePipelineIdx )
    : Pipeline( device )
  {
    /*vk::SpecializationInfo vSpecializationInfo(
      stage.specializationInfo->mapEntries.size( ),
      stage.specializationInfo->mapEntries.data( ),
      stage.specializationInfo->data.size( ),
      stage.specializationInfo->data.data( ) );*/
    vk::PipelineShaderStageCreateInfo vStage(
      {},
      stage.stage,
      stage.module ? static_cast< vk::ShaderModule >( *stage.module ) : nullptr,
      stage.name.data( ),
      nullptr//&vSpecializationInfo
    );

    setPipeline( vk::Device( *_device ).createComputePipeline(
      pipelineCache ? static_cast< vk::PipelineCache >( *pipelineCache ) : nullptr,
      vk::ComputePipelineCreateInfo(
        flags,
        vStage,
        layout ? static_cast< vk::PipelineLayout >( *layout ) : nullptr,
        basePipelineHandle ? static_cast< vk::Pipeline >( *basePipelineHandle ) : nullptr,
        basePipelineIdx
      ) ) );
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
    std::cerr << "Pipeline creation ..." << std::endl;
    std::vector<vk::SpecializationInfo> specializationInfos;
    specializationInfos.reserve( stages.size( ) );

    std::vector<vk::PipelineShaderStageCreateInfo> vStages;
    vStages.reserve( stages.size( ) );
    for ( auto const& s : stages )
    {
      if ( s.specializationInfo )
      {
        specializationInfos.push_back( vk::SpecializationInfo( 
          s.specializationInfo->mapEntries.size( ),
          s.specializationInfo->mapEntries.data( ),
          sizeof( s.specializationInfo->data ),
          s.specializationInfo->data )
        );
      }
      vStages.push_back( vk::PipelineShaderStageCreateInfo( { }, s.stage, 
        s.module ? static_cast<vk::ShaderModule>( *s.module ) : nullptr, 
        s.name.data( ),
        s.specializationInfo ? &specializationInfos.back( ) : nullptr )
      );
    }

    vk::PipelineVertexInputStateCreateInfo vVertexInputState;
    if ( vertexInputState )
    {
      vVertexInputState = vk::PipelineVertexInputStateCreateInfo( { },
        vertexInputState->vertexBindingDescriptions.size( ), 
        vertexInputState->vertexBindingDescriptions.data( ),
        vertexInputState->vertexAttrirDescriptions.size( ),
        vertexInputState->vertexAttrirDescriptions.data( )
      );
    }

    vk::PipelineViewportStateCreateInfo vViewportState;
    if ( viewportState )
    {
      vViewportState = vk::PipelineViewportStateCreateInfo( { }, 
        viewportState->viewports.size( ), viewportState->viewports.data( ),
        viewportState->scissors.size( ), viewportState->scissors.data( ) );
    }

    vk::PipelineMultisampleStateCreateInfo vMultisampleState;
    if ( multisampleState )
    {
      vMultisampleState = vk::PipelineMultisampleStateCreateInfo( { }, 
        multisampleState->rasterizationSamples, 
        multisampleState->sampleShadingEnable, multisampleState->minSampleShading,
        multisampleState->sampleMasks.empty( ) ? nullptr : 
          multisampleState->sampleMasks.data( ), 
        multisampleState->alphaToCoverageEnable,
        multisampleState->alphaToOneEnable );
    }

    vk::PipelineColorBlendStateCreateInfo vColorBlendState;
    if ( colorBlendState )
    {
      vColorBlendState = vk::PipelineColorBlendStateCreateInfo( { }, 
        colorBlendState->logicEnable, colorBlendState->logicOp,  
        colorBlendState->attachments.size( ),
        colorBlendState->attachments.data( ), colorBlendState->blendConstants );
    }

    vk::PipelineDynamicStateCreateInfo vDynamicState;
    if ( dynamicState )
    {
      vDynamicState = vk::PipelineDynamicStateCreateInfo( { }, 
        dynamicState->dynamicStates.size( ),
        dynamicState->dynamicStates.data( )
      );
    }

    vk::GraphicsPipelineCreateInfo pci(
      flags,
      vStages.size( ),
      vStages.data( ),
      vertexInputState ? &vVertexInputState : nullptr,
      inputAssemblyState,
      tessellationState,
      viewportState ? &vViewportState : nullptr,
      rasterizationState, multisampleState ? &vMultisampleState : nullptr,
      depthStencilState,
      colorBlendState ? &vColorBlendState : nullptr,
      dynamicState ? &vDynamicState : nullptr,
      pipelineLayout ? *pipelineLayout : vk::PipelineLayout( ),
      renderPass ? *renderPass : vk::RenderPass( ),
      subpass, basePipelineHandle ? *basePipelineHandle : vk::Pipeline( ),
      basePipelineIdx
    );
    std::cerr << " ... " << std::endl;
    setPipeline( static_cast<vk::Device>( *_device ).createGraphicsPipeline( 
      pipelineCache ? *pipelineCache : vk::PipelineCache( ), pci ) );
    std::cerr << " ... Pipeline created!" << std::endl;
  }


  PipelineLayout::PipelineLayout( const std::shared_ptr<Device>& device, 
    vk::ArrayProxy<const std::shared_ptr<DescriptorSetLayout>> setLayouts,
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

    vk::PipelineLayoutCreateInfo pci( {}, dsl.size( ), dsl.data( ), 
      pushConstantRanges.size( ), pushConstantRanges.data( ) );
    _pipelineLayout = static_cast< vk::Device >( *_device ).createPipelineLayout( pci );
  }

  PipelineLayout::~PipelineLayout( )
  {
    static_cast< vk::Device >( *_device ).destroyPipelineLayout( _pipelineLayout );
  }








  SpecializationInfo::SpecializationInfo( 
    vk::ArrayProxy<const vk::SpecializationMapEntry> mapEntries_, 
    const void* data_ )
    : mapEntries( mapEntries_.begin( ), mapEntries_.end( ) )
    , data( data_ )
  {
  }

  SpecializationInfo::SpecializationInfo( SpecializationInfo const& rhs )
    : SpecializationInfo( rhs.mapEntries, rhs.data )
  {
  }

  SpecializationInfo & SpecializationInfo::operator=( SpecializationInfo const& rhs )
  {
    mapEntries = rhs.mapEntries;
    return *this;
  }

  PipelineShaderStageCreateInfo::PipelineShaderStageCreateInfo( 
    vk::ShaderStageFlagBits stage_, const std::shared_ptr<ShaderModule>& module_, 
    const std::string& name_, vk::Optional<const SpecializationInfo> specializationInfo_ )
    : stage( stage_ )
    , module( module_ )
    , name( name_ )
    , specializationInfo( specializationInfo_ ? 
      new SpecializationInfo( *specializationInfo_ ) : nullptr )
  {
  }

  PipelineShaderStageCreateInfo::PipelineShaderStageCreateInfo( 
    const PipelineShaderStageCreateInfo& rhs )
    : PipelineShaderStageCreateInfo( rhs.stage, rhs.module, rhs.name, 
      rhs.specializationInfo ? vk::Optional<const SpecializationInfo>( 
        *rhs.specializationInfo.get( ) ) : nullptr )
  {
  }

  PipelineShaderStageCreateInfo & PipelineShaderStageCreateInfo::operator=( 
    const PipelineShaderStageCreateInfo& rhs )
  {
    stage = rhs.stage;
    module = rhs.module;
    name = rhs.name;
    specializationInfo.reset( rhs.specializationInfo ? new SpecializationInfo( *rhs.specializationInfo ) : nullptr );
    return *this;
  }
}