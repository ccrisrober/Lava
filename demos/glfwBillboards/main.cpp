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
#include "../utils/Camera.h"

#include <random>

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
  // camera
  Camera camera;

  struct PushConstant
  {
    float particleSize;
  } pc;

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 viewPos;
  } ubo;

  struct Vertex
  {
    glm::vec3 position;
  };

  std::vector<Vertex> vertices;

  std::shared_ptr<Buffer> uniformBuffer;
  std::shared_ptr<Buffer> vertexBuffer;
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr< Texture2DArray > tex;

  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
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

    std::mt19937 rndGenerator( time( nullptr ) );
    std::uniform_real_distribution<float> uniformDist( 0.0f, 1.0f );
    for ( uint32_t i = 0; i < 10000; ++i )
    {
      glm::vec3 vertex;
      vertex.x = 2000.0f * uniformDist( rndGenerator ) - 1000.0f;
      vertex.y = 2000.0f * uniformDist( rndGenerator ) - 1000.0f;
      vertex.z = 2000.0f * uniformDist( rndGenerator ) - 1000.0f;

      vertices.push_back( { vertex } );
    }

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, vertexBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }

    uniformBuffer = device->createUniformBuffer( sizeof( ubo ) );


    std::vector< std::string > djinnImages =
    {
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/djinn1.png" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/djinn2.png" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/djinn3.png" ),
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/djinn4.png" )
    };
    tex = device->createTexture2DArray( djinnImages,
      _window->gfxCommandPool( ), _window->gfxQueue( ),
      vk::Format::eR8G8B8A8Unorm );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    vk::PushConstantRange pushConstantRange(
      vk::ShaderStageFlagBits::eGeometry,
      0, sizeof( pc )
    );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout,
      pushConstantRange );

    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "billboard_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto geomStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "billboard_geom.spv" ),
      vk::ShaderStageFlagBits::eGeometry
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "billboard_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    vk::VertexInputBindingDescription binding(
      0, sizeof( Vertex ), vk::VertexInputRate::eVertex
    );

    PipelineVertexInputStateCreateInfo vertexInput( binding,
    {
      vk::VertexInputAttributeDescription( 0, 0,
      vk::Format::eR32G32B32Sfloat, 0
      )
    } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::ePointList, VK_FALSE
    );
    PipelineViewportStateCreateInfo viewport( 1, 1 ); // Dynamic viewport and scissors
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true, false,
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
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f
    );
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
    { vertexStage, geomStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( ) );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
    };
    auto dspPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( dspPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformBuffer, 0, sizeof( ubo ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler,
        1, tex->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void updateBuffers( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    glm::mat4 model;
    model = glm::rotate( model, time * glm::radians( 15.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    ubo.model = model;
    ubo.view = camera.GetViewMatrix( );

    ubo.projection = glm::perspective( glm::radians( camera.Zoom ),
      ( float ) width / ( float ) height, 1.0f, 100.0f );
    ubo.projection[ 1 ][ 1 ] *= -1;

    ubo.viewPos = camera.Position;

    uniformBuffer->set( &ubo );
  }

  void nextFrame( void ) override
  {
    updateBuffers( );

    pc.particleSize = 250.0f;

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
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
      pipelineLayout, 0, { descriptorSet }, { } );
    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );

    cmd->setViewportScissors( size );
    cmd->pushConstants<PushConstant>( pipelineLayout,
      vk::ShaderStageFlagBits::eGeometry, 0, pc );
    cmd->draw( vertices.size( ), 1, 0, 0 );
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
  VulkanWindow app( 500, 500, "GLFW Billboards", true );
  app.show( );
  return EXIT_SUCCESS;
}