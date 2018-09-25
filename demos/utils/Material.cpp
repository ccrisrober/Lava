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

#include "Material.h"

#include <glm/glm.hpp>

namespace pompeii
{
  Material::Material( void )
  {
  }
  void Material::bind( std::shared_ptr< CommandBuffer > cmd )
  {
    cmd->bindGraphicsPipeline( _pipeline );
  }

  void BasicTriangle::configure( const std::string& dir,
    std::shared_ptr< Device > dev, std::shared_ptr<RenderPass> renderPass )
  {
    struct Vertex
    {
      glm::vec4 position;
      glm::vec4 color;
    };

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
      dev->createDescriptorSetLayout( dslbs );
    _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, nullptr );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );

    PipelineShaderStageCreateInfo vertexStage = dev->createShaderPipelineShaderStage(
      dir + std::string( "triangle_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = dev->createShaderPipelineShaderStage(
      dir + std::string( "triangle_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding,
      {
        vk::VertexInputAttributeDescription(
          0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, position )
        ),
        vk::VertexInputAttributeDescription(
          1, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, color )
        )
      }
    );

    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, 
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } ); // Dynamic viewport and scissors
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true, false, 
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, 
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 
      0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, 
      stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport,
      vk::DynamicState::eScissor }
    );

    _pipeline = dev->createGraphicsPipeline( pipelineCache, {}, 
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, renderPass
    );
  }
  void BasicTessTriangle::configure( const std::string& dir,
    std::shared_ptr< Device > dev, std::shared_ptr<RenderPass> renderPass )
  {
    struct Vertex
    {
      glm::vec4 position;
      glm::vec4 color;
    };

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
      dev->createDescriptorSetLayout( dslbs );
    _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, nullptr );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      
    PipelineShaderStageCreateInfo vertexStage = dev->createShaderPipelineShaderStage(
      dir + std::string( "tesstriangle_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo ctrlStage = dev->createShaderPipelineShaderStage(
      dir + std::string( "tesstriangle_tesc.spv" ),
      vk::ShaderStageFlagBits::eTessellationControl
    );
    PipelineShaderStageCreateInfo evalStage = dev->createShaderPipelineShaderStage(
      dir + std::string( "tesstriangle_tese.spv" ),
      vk::ShaderStageFlagBits::eTessellationEvaluation
    );
    PipelineShaderStageCreateInfo fragmentStage = dev->createShaderPipelineShaderStage(
      dir + std::string( "tesstriangle_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding,
      {
        vk::VertexInputAttributeDescription(
          0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, position )
        ),
        vk::VertexInputAttributeDescription(
          1, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, color )
        )
      }
    );

    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, 
      vk::PrimitiveTopology::ePatchList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } ); // Dynamic viewport and scissors
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true, false, 
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, 
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );

    rasterization.polygonMode = vk::PolygonMode::eLine;
    rasterization.lineWidth = 1.0f;

    rasterization.cullMode = vk::CullModeFlagBits::eNone;

    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 
      0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, 
      stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport,
      vk::DynamicState::eScissor }
    );

    vk::PipelineTessellationStateCreateInfo tessState( {}, 3 );

    _pipeline = dev->createGraphicsPipeline( pipelineCache, { }, 
      { vertexStage, fragmentStage, ctrlStage, evalStage }, 
      vertexInput, assembly, tessState, 
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, renderPass
    );
  }
}