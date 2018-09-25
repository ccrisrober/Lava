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
using namespace pompeii;

#include <routes.h>
#include <glm/glm.hpp>

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }
  void initResources( void ) override
  {
    auto device = _window->device( );
    auto devProps = _window->physicalDeviceProperties( );

    const glm::vec3 texels[ ] = {
      glm::vec3( 1.0f, 0.0f, 0.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ),
      glm::vec3( 0.0f, 0.0f, 1.0f )
    };

    if ( devProps.limits.maxTexelBufferElements < sizeof( texels ) )
    {
      std::cout << "maxTexelBufferElements too small" << std::endl;
      exit( -1 );
    }
    auto formatProps = _window->physicalDevice( )->
      getFormatProperties( vk::Format::eR32G32B32Sfloat );

    if ( !( formatProps.bufferFeatures &
      vk::FormatFeatureFlagBits::eUniformTexelBuffer ) )
    {
      std::cout << "R32G32B32_SFLOAT format unsupported for texel buffer"
        << std::endl;
      exit( -1 );
    }

    // Texel buffer
    {
      texelBuffer = device->createBuffer( sizeof( texels ),
        vk::BufferUsageFlagBits::eUniformTexelBuffer,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    texelBuffer->set( &texels );

    texelView = device->createBufferView( texelBuffer,
      vk::Format::eR32G32B32Sfloat,
      0, sizeof( texels ) );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformTexelBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };

    auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    auto descriptorPool = device->createDescriptorPool( 1, {
      // Binding 0: Texel buffer
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformTexelBuffer, 1 )
    } );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool,
      descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformTexelBuffer,
        1, nullptr,
        DescriptorBufferInfo(
          texelBuffer, 0, sizeof( texels )
        ), texelView
      )
    };
    device->updateDescriptorSets( wdss, { } );



    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "texelBuffer_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "texelBuffer_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );
    PipelineMultisampleStateCreateInfo multisample(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep,
      vk::CompareOp::eAlways, 0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { },
      true, true, vk::CompareOp::eLessOrEqual, false, false,
      stencilOpState, stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false,
      vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend,
      dynamic, pipelineLayout, _window->renderPass( ) );
  }

  void nextFrame( void ) override
  {
    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    float _red = sin( time ) * 0.5f + 0.5f;
    float _blue = cos( time ) * 0.5f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { _red, 0.0f, _blue, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    auto size = _window->swapchainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = size;
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );
    cmd->bindGraphicsPipeline( pipeline );

    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    cmd->setViewportScissors( size );

    cmd->draw( 3, 1, 0, 0 );
    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<Buffer> texelBuffer;
  std::shared_ptr<BufferView> texelView;
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
  VulkanWindow app( 500, 500, "GLFW Texel Buffer", true );
  app.show( );
  return EXIT_SUCCESS;
}