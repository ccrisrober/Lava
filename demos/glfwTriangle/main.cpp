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

#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
#include <glm/glm.hpp>
using namespace pompeii;

#include <routes.h>

#define INDEXING_MODE // Comment for not indexing mode

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  struct Vertex
  {
    glm::vec2 position;
    glm::vec3 color;
  };

  const std::vector<Vertex> vertices =
  {
    { {  0.75f,  0.75f }, { 1.0f, 0.0f, 0.0f } },
    { { -0.75f,  0.75f }, { 0.0f, 1.0f, 0.0f } },
    { {   0.0f, -0.75f }, { 0.0f, 0.0f, 1.0f } }
  };
#ifdef INDEXING_MODE
  const std::vector<uint32_t> indices = { 0, 1, 2 };
#endif

  void initResources( void ) override
  {
    auto device = _window->device( );
    auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
    cmd->begin( );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );

      vertexBuffer = device->createBuffer( vertexBufferSize, 
        vk::BufferUsageFlagBits::eVertexBuffer | 
        vk::BufferUsageFlagBits::eTransferDst, 
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ), 
        vertices.data( ) } );
    }

#ifdef INDEXING_MODE
    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      indexBuffer->update<uint32_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
    }
#endif
    cmd->end( );
    _window->gfxQueue( )->submitAndWait( cmd );

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
      device->createDescriptorSetLayout( dslbs );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "triangle_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "triangle_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), 
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding,
      {
        vk::VertexInputAttributeDescription(
          0, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, position )
        ),
        vk::VertexInputAttributeDescription(
          1, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, color )
        )
      }
    );

    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, 
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
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

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), {}, 
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( )
    );
  }

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 0.0f, 0.0f, 0.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->bindGraphicsPipeline( pipeline );

    cmd->bindVertexBuffer( 0, vertexBuffer );
#ifdef INDEXING_MODE
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint32 );
#endif

    cmd->setViewportScissors( extent );
#ifdef INDEXING_MODE
    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );
#else
    cmd->draw( uint32_t( vertices.size( ) ), 1, 0, 0 );
#endif

    cmd->endRenderPass( );

    _window->frameReady( );
  }

private:
  std::shared_ptr<Buffer> vertexBuffer;
#ifdef INDEXING_MODE
  std::shared_ptr<Buffer> indexBuffer;
#endif
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500,
#ifdef INDEXING_MODE
    "GLFW Indexed Triangle",
#else
    "GLFW Triangle",
#endif
    true );
  app.show( );
  return EXIT_SUCCESS;
}