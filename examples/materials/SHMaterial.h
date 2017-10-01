#pragma once

#include <lava/lava.h>

using namespace lava;

namespace material
{
  class SHMaterial: public lava::engine::Material
  {
  public:
    virtual void configure( const std::string& dir,
      std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass )
    {
      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs;
      DescriptorSetLayoutBinding mvpDescriptor( 0, vk::DescriptorType::eUniformBuffer, 
        vk::ShaderStageFlagBits::eVertex );
      dslbs.push_back( mvpDescriptor );
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = dev->createDescriptorSetLayout( dslbs );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, nullptr );

      std::array<vk::DescriptorPoolSize, 1> poolSize;
      poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
      std::shared_ptr<DescriptorPool> descriptorPool = dev->createDescriptorPool( {}, 1, poolSize );

      // Init descriptor set
      _descriptorSet = dev->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
      std::vector<WriteDescriptorSet> wdss;
      DescriptorBufferInfo buffInfo( _uniformBufferMVP, 0, sizeof( glm::mat4 ) );
      WriteDescriptorSet w( _descriptorSet, 0, 0, 
        vk::DescriptorType::eUniformBuffer, 1, nullptr, buffInfo );
      wdss.push_back( w );
      dev->updateDescriptorSets( wdss, {} );

      // init shaders
      std::shared_ptr<ShaderModule> vertexShaderModule = dev->createShaderModule( 
        dir + std::string( "spharmonics_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
      std::shared_ptr<ShaderModule> fragmentShaderModule = dev->createShaderModule( 
        dir + std::string( "spharmonics_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
      PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );

      PipelineVertexInputStateCreateInfo vertexInput( 
        vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ), 
          vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof(lava::extras::Vertex, position ) ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof(lava::extras::Vertex, normal ) )
        }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );   // one dummy viewport and scissor, as dynamic state sets them
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true, 
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, 
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, 
        false, 0.0f, nullptr, false, false );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, 
        vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, 
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
      vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, 
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
        vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
        colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
      PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, 
        vk::DynamicState::eScissor } );


      _pipeline = dev->createGraphicsPipeline( pipelineCache, {}, { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        _pipelineLayout, _renderPass );
      }
  };
}