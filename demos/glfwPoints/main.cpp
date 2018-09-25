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

#include <random>

#include <routes.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VERTEX_BUFFER_POS_ID 0
#define VERTEX_BUFFER_COL_ID 1

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
    glm::mat4 modelViewProjection;
  } ubo;
  
  uint32_t particles = 500000;

  std::shared_ptr< Buffer > positionBuffer, colorBuffer;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr<UniformBuffer> uboBuffer;

  virtual void initResources( void ) override
  {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen( rd( ) ); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis( 0.0f, 1.0f );

    uint32_t n = 1000,
             n2 = n / 2; // particles spread in the cube

    std::vector<glm::vec3> positions;
    positions.reserve( particles );
    std::vector<glm::vec3> colors;
    colors.reserve( particles );

    for ( uint32_t i = 0; i < particles; ++i )
    {
      // positions
      float x = dis( gen ) * n - n2,
        y = dis( gen ) * n - n2,
        z = dis( gen ) * n - n2;
      positions.push_back( glm::vec3( x, y, z ) );

      // colors
      float vx = ( x / n ) + 0.5f,
            vy = ( y / n ) + 0.5f,
            vz = ( z / n ) + 0.5f;
      colors.push_back( glm::vec3( vx, vy, vz ) );
    }

    auto device = _window->device( );

    uboBuffer = device->createUniformBuffer( sizeof( ubo ) );


    // Position vertex buffer
    {
      uint32_t vertexBufferSize = positions.size( ) * sizeof( glm::vec3 );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, positions.data( ) );

      positionBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, positionBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }
    // Color vertex buffer
    {
      uint32_t vertexBufferSize = colors.size( ) * sizeof( glm::vec3 );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, colors.data( ) );

      colorBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, positionBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    auto descriptorPool = device->createDescriptorPool( 1, {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 )
    } );

    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uboBuffer, 0, sizeof( ubo )
        )
      )
    };

    device->updateDescriptorSets( wdss, { } );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "points_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "points_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput( {
      // Binding point 0: Position layout description at per-vertex rate
      vk::VertexInputBindingDescription( VERTEX_BUFFER_POS_ID,
        sizeof( glm::vec3 ), vk::VertexInputRate::eVertex
      ),
      // Binding point 1: Color layout description at per-vertex rate
      vk::VertexInputBindingDescription( VERTEX_BUFFER_COL_ID,
        sizeof( glm::vec3 ), vk::VertexInputRate::eVertex
      )
    }, {
      // Position attribute
      vk::VertexInputAttributeDescription( 0, VERTEX_BUFFER_POS_ID,
        vk::Format::eR32G32B32Sfloat, 0
      ),
      // Color attribute
      vk::VertexInputAttributeDescription( 1, VERTEX_BUFFER_COL_ID,
        vk::Format::eR32G32B32Sfloat, 0
      )
    } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::ePointList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );
    ;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( true,
      vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
      vk::BlendFactor::eOne, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( )
    );
  }

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;

    vk::Extent2D extent = _window->swapchainImageSize( );

    glm::vec3 cameraPos( 0.0f, 0.0f, 2750.0f );

    glm::mat4 view = glm::lookAt( cameraPos, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    glm::mat4 projection = glm::perspective( 27.0f, extent.width / ( float ) extent.height, 5.0f, 3500.0f );
    
    /*// Vulkan clip space has inverted Y and half Z.
    glm::mat4 clip = glm::mat4(
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.0f, 0.0f, 0.5f, 1.0f
    );
    projection = clip * projection;*/
    projection[ 1 ][ 1 ] *= -1;


    glm::mat4 model( 1.0f );

    model = glm::rotate( model, time * 0.25f, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    model = glm::rotate( model, time * 0.5f, glm::vec3( 0.0f, 1.0f, 0.0f ) );

    ubo.modelViewProjection = projection * model;
    uboBuffer->set( &ubo );

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 0.0f, 0.0f, 0.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipelineLayout,
      0, { descriptorSet }, { } );

    // Binding point 0 : Position vertex buffer
    cmd->bindVertexBuffer( VERTEX_BUFFER_POS_ID, positionBuffer, 0 );
    // Binding point 1 : Color vertex buffer
    cmd->bindVertexBuffer( VERTEX_BUFFER_COL_ID, positionBuffer, 0 );

    cmd->setViewportScissors( extent );
    cmd->draw( particles );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
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
  VulkanWindow app( 500, 500, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}