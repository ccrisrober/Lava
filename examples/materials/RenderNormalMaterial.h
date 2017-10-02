#pragma once

#include <lava/lava.h>

using namespace lava;

namespace material
{
  class RenderNormalMaterial: public lava::engine::Material
  {
  public:
    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
    } uboVS;

    struct Vertex
    {
      glm::vec3 pos;
      glm::vec3 color;
    };
    virtual void configure( const std::string& dir,
      std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass )
    {
      // MVP buffer
      {
        uint32_t mvpBufferSize = sizeof( uboVS );
        _uniformBufferMVP = dev->createBuffer( mvpBufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }

      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs = 
      {
        DescriptorSetLayoutBinding( 0, 
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex
        )
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
        dev->createDescriptorSetLayout( dslbs );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, nullptr );

      std::array<vk::DescriptorPoolSize, 1> poolSize =
      {
        // Binding 0: MVP uniform buffer
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
      };
      std::shared_ptr<DescriptorPool> descriptorPool = 
        dev->createDescriptorPool( {}, 1, poolSize );


      // Init descriptor set
      _descriptorSet = dev->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet( _descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( _uniformBufferMVP, 0, sizeof( uboVS ) ) )
      };
      dev->updateDescriptorSets( wdss, {} );


      // init shaders
      std::shared_ptr<ShaderModule> vertexShaderModule =
        dev->createShaderModule( dir + std::string( "cube_vert.spv" ), 
          vk::ShaderStageFlagBits::eVertex
        );
      std::shared_ptr<ShaderModule> fragmentShaderModule =
        dev->createShaderModule( dir + std::string( "cube_frag.spv" ), 
          vk::ShaderStageFlagBits::eFragment
        );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = 
        dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage( 
        vk::ShaderStageFlagBits::eVertex, vertexShaderModule
      );
      PipelineShaderStageCreateInfo fragmentStage( 
        vk::ShaderStageFlagBits::eFragment, fragmentShaderModule
      );
      vk::VertexInputBindingDescription binding( 0, 
        sizeof( Vertex ), vk::VertexInputRate::eVertex
      );

      PipelineVertexInputStateCreateInfo vertexInput( binding,
        {
          vk::VertexInputAttributeDescription( 0, 0, 
            vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos )
          ),
          vk::VertexInputAttributeDescription( 1, 0, 
            vk::Format::eR32G32B32Sfloat, offsetof( Vertex, color )
          )
        }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
      vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
      PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );


      _pipeline = dev->createGraphicsPipeline( pipelineCache, { }, { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        _pipelineLayout, renderPass );
    }
    virtual void bind( std::shared_ptr< CommandBuffer > cmd )
    {
      lava::engine::Material::bind( cmd );
      if ( _descriptorSet )
      {
        cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
          _pipelineLayout, 0, { _descriptorSet }, nullptr );
      }
    }
    std::shared_ptr<Buffer> _uniformBufferMVP;
  protected:
    std::shared_ptr<DescriptorSet> _descriptorSet;
  };
}