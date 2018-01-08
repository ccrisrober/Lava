#include "VulkanPipelineState.h"
#include "VulkanRenderAPI.h"
#include <assert.h>

namespace lava
{
  TGraphicsPipelineState::TGraphicsPipelineState( const StateDescType& desc )
  {
    _data = desc;
  }


  VulkanShaderModule::VulkanShaderModule( VulkanDevicePtr device,
    VkShaderModule module )
    : VulkanResource( device )
    , _module( module )
  {
  }
  VulkanShaderModule::~VulkanShaderModule( void )
  {
    vkDestroyShaderModule( _device->getLogical( ), _module, nullptr );
  }

  VulkanGpuProgram::~VulkanGpuProgram( void )
  {
    delete _module;
  }

  VulkanGraphicsPipelineState::VulkanGraphicsPipelineState( VulkanDevicePtr device,
    const StateDescType& desc )
    : TGraphicsPipelineState( desc )
    , _device( device )
  {
    std::pair< VkShaderStageFlagBits, GpuProgram* > stages[] =
    {
      { VK_SHADER_STAGE_VERTEX_BIT, _data.vertexProgram },
      { VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, _data.tessCtrlProgram },
      { VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, _data.tessEvalProgram },
      { VK_SHADER_STAGE_GEOMETRY_BIT, _data.geometryProgram },
      { VK_SHADER_STAGE_FRAGMENT_BIT, _data.fragmentProgram }
    };

    uint32_t stageOutputIdx = 0;
    uint32_t numStages = sizeof( stages ) / sizeof( stages[ 0 ] );
    for ( uint32_t i = 0; i < numStages; ++i )
    {
      VulkanGpuProgram* program = static_cast< VulkanGpuProgram* >( stages[ i ].second );
      if ( program == nullptr ) continue;

      VkPipelineShaderStageCreateInfo& stageCI = _shaderStageInfos[ stageOutputIdx ];
      stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stageCI.pNext = nullptr;
      stageCI.flags = 0;
      stageCI.stage = stages[ i ].first;
      stageCI.module = VK_NULL_HANDLE;
      stageCI.pName = "main";
      stageCI.pSpecializationInfo = nullptr;

      ++stageOutputIdx;
    }

    uint32_t numUsedStages = stageOutputIdx;

    bool tessEnabled = _data.tessCtrlProgram != nullptr &&
      _data.tessEvalProgram != nullptr;

    _inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    _inputAssemblyInfo.pNext = nullptr;
    _inputAssemblyInfo.flags = 0;
    _inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    _inputAssemblyInfo.primitiveRestartEnable = false;

    _tesselationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    _tesselationInfo.pNext = nullptr;
    _tesselationInfo.flags = 0;
    _tesselationInfo.patchControlPoints = 3;

    _viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    _viewportInfo.pNext = nullptr;
    _viewportInfo.flags = 0;
    _viewportInfo.viewportCount = 1; // Spec says this need to be at least 1...
    _viewportInfo.scissorCount = 1;
    _viewportInfo.pViewports = nullptr; // Dynamic
    _viewportInfo.pScissors = nullptr; // Dynamic

    _rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _rasterizationInfo.pNext = nullptr;
    _rasterizationInfo.flags = 0;
    _rasterizationInfo.depthClampEnable = VK_FALSE; // !rstProps.getDepthClipEnable( );
    _rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    _rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL; // VulkanUtility::getPolygonMode( rstProps.getPolygonMode( ) );
    _rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT; // VulkanUtility::getCullMode( rstProps.getCullMode( ) );
    _rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    _rasterizationInfo.depthBiasEnable = VK_FALSE; // rstProps.getDepthBias( ) != 0.0f;
    _rasterizationInfo.depthBiasConstantFactor = 0.0f; // rstProps.getDepthBias( );
    _rasterizationInfo.depthBiasSlopeFactor = 0.0f; // rstProps.getSlopeScaledDepthBias( );
    _rasterizationInfo.depthBiasClamp = 0.0f; // _rasterizationInfo.depthClampEnable ? rstProps.getDepthBiasClamp( ) : 0.0f;
    _rasterizationInfo.lineWidth = 1.0f;


    _multiSampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    _multiSampleInfo.pNext = nullptr;
    _multiSampleInfo.flags = 0;
    _multiSampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Assigned at runtime
    _multiSampleInfo.sampleShadingEnable = VK_FALSE; // When enabled, perform shading per sample instead of per pixel (more expensive, essentially FSAA)
    _multiSampleInfo.minSampleShading = 1.0f; // Minimum percent of samples to run full shading for when sampleShadingEnable is enabled (1.0f to run for all)
    _multiSampleInfo.pSampleMask = nullptr; // Normally one bit for each sample: e.g. 0x0000000F to enable all samples in a 4-sample setup
    _multiSampleInfo.alphaToCoverageEnable = VK_FALSE; // blendProps.getAlphaToCoverageEnabled( );
    _multiSampleInfo.alphaToOneEnable = VK_FALSE;

    /*VkStencilOpState stencilFrontInfo;
    stencilFrontInfo.compareOp = VulkanUtility::getCompareOp( dsProps.getStencilFrontCompFunc( ) );
    stencilFrontInfo.depthFailOp = VulkanUtility::getStencilOp( dsProps.getStencilFrontZFailOp( ) );
    stencilFrontInfo.passOp = VulkanUtility::getStencilOp( dsProps.getStencilFrontPassOp( ) );
    stencilFrontInfo.failOp = VulkanUtility::getStencilOp( dsProps.getStencilFrontFailOp( ) );
    stencilFrontInfo.reference = 0; // Dynamic
    stencilFrontInfo.compareMask = ( UINT32 ) dsProps.getStencilReadMask( );
    stencilFrontInfo.writeMask = ( UINT32 ) dsProps.getStencilWriteMask( );

    VkStencilOpState stencilBackInfo;
    stencilBackInfo.compareOp = VulkanUtility::getCompareOp( dsProps.getStencilBackCompFunc( ) );
    stencilBackInfo.depthFailOp = VulkanUtility::getStencilOp( dsProps.getStencilBackZFailOp( ) );
    stencilBackInfo.passOp = VulkanUtility::getStencilOp( dsProps.getStencilBackPassOp( ) );
    stencilBackInfo.failOp = VulkanUtility::getStencilOp( dsProps.getStencilBackFailOp( ) );
    stencilBackInfo.reference = 0; // Dynamic
    stencilBackInfo.compareMask = ( UINT32 ) dsProps.getStencilReadMask( );
    stencilBackInfo.writeMask = ( UINT32 ) dsProps.getStencilWriteMask( );*/

    _depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    _depthStencilInfo.pNext = nullptr;
    _depthStencilInfo.flags = 0;
    _depthStencilInfo.depthBoundsTestEnable = false;
    _depthStencilInfo.minDepthBounds = 0.0f;
    _depthStencilInfo.maxDepthBounds = 1.0f;
    _depthStencilInfo.depthTestEnable = VK_TRUE; // dsProps.getDepthReadEnable( );
    _depthStencilInfo.depthWriteEnable = VK_TRUE; // dsProps.getDepthWriteEnable( );
    _depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS; // VulkanUtility::getCompareOp( dsProps.getDepthComparisonFunc( ) );
    _depthStencilInfo.front = { }; // stencilFrontInfo;
    _depthStencilInfo.back = { }; //stencilBackInfo;
    _depthStencilInfo.stencilTestEnable = VK_FALSE; // dsProps.getStencilEnable( );


    /*VkPipelineColorBlendAttachmentState& blendState = mAttachmentBlendStates[ i ];
    blendState.blendEnable = blendProps.getBlendEnabled( rtIdx );
    blendState.colorBlendOp = VulkanUtility::getBlendOp( blendProps.getBlendOperation( rtIdx ) );
    blendState.srcColorBlendFactor = VulkanUtility::getBlendFactor( blendProps.getSrcBlend( rtIdx ) );
    blendState.dstColorBlendFactor = VulkanUtility::getBlendFactor( blendProps.getDstBlend( rtIdx ) );
    blendState.alphaBlendOp = VulkanUtility::getBlendOp( blendProps.getAlphaBlendOperation( rtIdx ) );
    blendState.srcAlphaBlendFactor = VulkanUtility::getBlendFactor( blendProps.getAlphaSrcBlend( rtIdx ) );
    blendState.dstAlphaBlendFactor = VulkanUtility::getBlendFactor( blendProps.getAlphaDstBlend( rtIdx ) );
    blendState.colorWriteMask = blendProps.getRenderTargetWriteMask( rtIdx ) & 0xF;*/

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { };
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    _colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _colorBlendStateInfo.pNext = nullptr;
    _colorBlendStateInfo.flags = 0;
    _colorBlendStateInfo.logicOpEnable = VK_FALSE;
    _colorBlendStateInfo.logicOp = VK_LOGIC_OP_NO_OP;
    _colorBlendStateInfo.attachmentCount = 0; // Assigned at runtime
    _colorBlendStateInfo.pAttachments = &colorBlendAttachment; // mAttachmentBlendStates;
    _colorBlendStateInfo.blendConstants[ 0 ] = 0.0f;
    _colorBlendStateInfo.blendConstants[ 1 ] = 0.0f;
    _colorBlendStateInfo.blendConstants[ 2 ] = 0.0f;
    _colorBlendStateInfo.blendConstants[ 3 ] = 0.0f;

    _dynamicStates[ 0 ] = VK_DYNAMIC_STATE_VIEWPORT;
    _dynamicStates[ 1 ] = VK_DYNAMIC_STATE_SCISSOR;
    _dynamicStates[ 2 ] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;

    uint32_t numDynamicStates = sizeof( _dynamicStates ) / sizeof( _dynamicStates[ 0 ] );
    assert( numDynamicStates == 3 );

    _dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    _dynamicStateInfo.pNext = nullptr;
    _dynamicStateInfo.flags = 0;
    _dynamicStateInfo.dynamicStateCount = numDynamicStates;
    _dynamicStateInfo.pDynamicStates = _dynamicStates;


    _pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    _pipelineInfo.pNext = nullptr;
    _pipelineInfo.flags = 0;
    _pipelineInfo.stageCount = numUsedStages;
    _pipelineInfo.pStages = _shaderStageInfos;
    _pipelineInfo.pVertexInputState = nullptr; // Assigned at runtime
    _pipelineInfo.pInputAssemblyState = &_inputAssemblyInfo;
    _pipelineInfo.pTessellationState = tessEnabled ? &_tesselationInfo : nullptr;
    _pipelineInfo.pViewportState = &_viewportInfo;
    _pipelineInfo.pRasterizationState = &_rasterizationInfo;
    _pipelineInfo.pMultisampleState = &_multiSampleInfo;
    _pipelineInfo.pDepthStencilState = nullptr; // Assigned at runtime
    _pipelineInfo.pColorBlendState = nullptr; // Assigned at runtime
    _pipelineInfo.pDynamicState = &_dynamicStateInfo;
    _pipelineInfo.renderPass = VK_NULL_HANDLE; // Assigned at runtime
    _pipelineInfo.layout = VK_NULL_HANDLE; // Assigned at runtime
    _pipelineInfo.subpass = 0;
    _pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    _pipelineInfo.basePipelineIndex = -1;


  }


  VulkanPipeline* VulkanGraphicsPipelineState::createPipeline( )
  {

    std::pair<VkShaderStageFlagBits, GpuProgram*> stages[] =
    {
      { VK_SHADER_STAGE_VERTEX_BIT, _data.vertexProgram },
      { VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, _data.tessCtrlProgram },
      { VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, _data.tessEvalProgram },
      { VK_SHADER_STAGE_GEOMETRY_BIT, _data.geometryProgram },
      { VK_SHADER_STAGE_FRAGMENT_BIT, _data.fragmentProgram }
    };

    uint32_t stageOutputIdx = 0;
    uint32_t numStages = sizeof( stages ) / sizeof( stages[ 0 ] );
    for ( uint32_t i = 0; i < numStages; ++i )
    {
      VulkanGpuProgram* program = static_cast< VulkanGpuProgram* >( stages[ i ].second );
      if ( program == nullptr )
        continue;

      VkPipelineShaderStageCreateInfo& stageCI = _shaderStageInfos[ stageOutputIdx ];

      VulkanShaderModule* module = program->getShaderModule( );

      if ( module != nullptr )
        stageCI.module = module->getHandle( );
      else
        stageCI.module = VK_NULL_HANDLE;

      ++stageOutputIdx;
    }

    VkPipeline pip;
    VkResult result = vkCreateGraphicsPipelines( _device->getLogical( ),
      VK_NULL_HANDLE, 1, &_pipelineInfo, nullptr, &pip );
    assert( result == VK_SUCCESS );

    return new VulkanPipeline( _device, pip );
  }


  VulkanComputePipelineState::VulkanComputePipelineState( )
  {
    VkPipelineShaderStageCreateInfo stageCI;
    stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCI.pNext = nullptr;
    stageCI.flags = 0;
    stageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageCI.module = VK_NULL_HANDLE;
    stageCI.pName = "main";
    stageCI.pSpecializationInfo = nullptr;

    VkComputePipelineCreateInfo pipCI;
    pipCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipCI.pNext = nullptr;
    pipCI.flags = 0;
    pipCI.stage = stageCI;
    pipCI.basePipelineHandle = VK_NULL_HANDLE;
    pipCI.basePipelineIndex = -1;

    auto api = VulkanRenderAPI::getInstance( );

    _device = api->_getPresentDevice( );

    VkPipeline pipeline;
    VkResult result = vkCreateComputePipelines( _device->getLogical( ),
      VK_NULL_HANDLE, 1, &pipCI, nullptr, &pipeline );
    assert( result == VK_SUCCESS );

    _pipeline = new VulkanPipeline( _device, pipeline );
  }
  VulkanComputePipelineState::~VulkanComputePipelineState( void )
  {
    if ( !_device ) return;
    delete _pipeline;
  }
  VulkanPipeline* VulkanComputePipelineState::getPipeline( /*uint32_t deviceIdx*/ ) const
  {
    return _pipeline;
  }
  VkPipelineLayout VulkanComputePipelineState::getPipelineLayout( /*uint32_t deviceIdx*/ ) const
  {
    return _pipelineLayout;
  }
}

#include "VulkanRenderAPI.h"

namespace lava
{
  void VulkanGpuProgram::initialize( void )
  {
    std::vector<char>& code = readFile( _desc.file );
    VkShaderModuleCreateInfo modCI;
    modCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    modCI.pNext = nullptr;
    modCI.flags = 0;
    modCI.codeSize = code.size( );
    modCI.pCode = reinterpret_cast< const uint32_t * > ( code.data( ) );

    auto api = VulkanRenderAPI::getInstance( );

    VulkanDevicePtr device = api->_getPresentDevice( );

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule( device->getLogical( ), &modCI, nullptr, &shaderModule );
    assert( result == VK_SUCCESS );
    
    _module = new VulkanShaderModule( device, shaderModule );

    _isCompiled = true;

  }
}