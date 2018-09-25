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
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    float time;
  } uboVS;

  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "monkey.obj_" ) );

    // MVP buffer
    {
      mvpBuffer = device->createUniformBuffer( sizeof( uboVS ) );
    }

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "explosion_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );

    auto geomStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "explosion_geom.spv" ),
      vk::ShaderStageFlagBits::eGeometry
    );

    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "explosion_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( pompeii::utility::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription(
            0, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( pompeii::utility::Vertex, position )
          ),
          vk::VertexInputAttributeDescription(
            1, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( pompeii::utility::Vertex, normal )
          ),
          vk::VertexInputAttributeDescription(
            2, 0, vk::Format::eR32G32Sfloat,
            offsetof( pompeii::utility::Vertex, texCoord )
          )
        }
    );

    vk::PipelineInputAssemblyStateCreateInfo assembly( {},
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );
    ;
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      vk::PipelineColorBlendAttachmentState(
        false, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
      ), { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
      { vertexStage, geomStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( )
    );

    std::array<vk::DescriptorPoolSize, 1> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
    };
    auto dspPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( dspPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( mvpBuffer, 0, sizeof( uboVS ) )
      )
    };
    device->updateDescriptorSets( wdss, {} );

    pipelineStatNames = {
      "Input assembly vertex count        ",
      "Input assembly primitives count    ",
      "Vertex shader invocations          ",
      "Clipping stage primitives processed",
      "Clipping stage primtives output    ",
      "Fragment shader invocations        "
    };
    pipelineStats.resize( pipelineStatNames.size( ) );
    query = device->createPipelineStatisticsQuery( 6,
      vk::QueryPipelineStatisticFlagBits::eInputAssemblyVertices |
      vk::QueryPipelineStatisticFlagBits::eInputAssemblyPrimitives |
      vk::QueryPipelineStatisticFlagBits::eVertexShaderInvocations |
      vk::QueryPipelineStatisticFlagBits::eClippingInvocations |
      vk::QueryPipelineStatisticFlagBits::eClippingPrimitives |
      vk::QueryPipelineStatisticFlagBits::eFragmentShaderInvocations );
  }

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), 
      time * 0.5f * glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f )
    );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 0.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uboVS.time = time;

    mvpBuffer->set( &uboVS );
  }

  void nextFrame( void ) override
  {
    getQueryResults( );
    
    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.3f, 0.2f, 0.8f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    auto cmd = _window->currentCommandBuffer( );

    // Reset timestamp query pool
    cmd->resetQueryPool( query, 0, pipelineStats.size( ) );

      const auto size = _window->swapchainImageSize( );
      vk::Rect2D rect;
      rect.extent = size;
      cmd->beginRenderPass(
        _window->renderPass( ),
        _window->framebuffer( ),
        rect, clearValues, vk::SubpassContents::eInline
      );

      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayout, 0, { descriptorSet }, nullptr );

      cmd->setViewportScissors( size );

      // Start capture of pipeline statistics
      cmd->beginQuery( query, 0, vk::QueryControlFlagBits::ePrecise );
    
      cmd->bindGraphicsPipeline( pipeline );
      geometry->render( cmd );

    // End capture of pipeline statistics
    cmd->endQuery( query, 0 );

      cmd->endRenderPass( );

      _window->frameReady( );
  }
private:
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;

  std::shared_ptr< Pipeline > pipeline;

  std::shared_ptr< pompeii::utility::Geometry > geometry;
  std::shared_ptr< UniformBuffer > mvpBuffer;

  void getQueryResults( void )
  {
    uint32_t count = static_cast<uint32_t>( pipelineStats.size( ) );
    pipelineStats = query->getResults<uint64_t>( 0, 1, count,
      sizeof( uint64_t ), vk::QueryResultFlagBits::e64 );
    if ( !pipelineStats.empty( ) )
    {
      for ( uint32_t i = 0; i < pipelineStats.size( ); ++i )
      {
        std::cout << pipelineStatNames[ i ] << ": " << pipelineStats[ i ] << std::endl;
      }
    }
    std::cout << "====================================" << std::endl;
  }

  // Vector for storing pipeline statistics results
  std::vector<uint64_t> pipelineStats;
  std::vector<std::string> pipelineStatNames;
  std::shared_ptr<pompeii::QueryPool> query;
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {

  }
  /*VulkanWindow( int w, int h, const char* title, bool enableLayers )
    : glfw::VulkanWindow( w, h, title, enableLayers )
  {

  }*/
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "Mesh Explosion", true );
  app.show( );
  return EXIT_SUCCESS;
}