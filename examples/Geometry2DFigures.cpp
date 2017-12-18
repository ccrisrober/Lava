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

#include <lava/lava.h>
using namespace lava;

#include <routes.h>

struct Vertex
{
  glm::vec2 position;
  glm::vec3 color;
  uint32_t sides;
};

std::vector<Vertex> vertices =
{
  //   Coordinates             Color        Sides
  { { -0.45f,  0.45f }, { 1.0f, 0.0f, 0.0f },  4 },
  { {  0.45f,  0.45f }, { 0.0f, 1.0f, 0.0f },  8 },
  { {  0.45f, -0.45f }, { 0.0f, 0.0f, 1.0f }, 16 },
  { { -0.45f, -0.45f }, { 1.0f, 1.0f, 0.0f }, 32 }
};

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Buffer> vertexBuffer;
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<CommandPool> commandPool;

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    // create a command pool for command buffer allocation
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
    vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
    vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

    // Init descriptor and pipeline layouts
    auto descriptorSetLayout = _device->createDescriptorSetLayout( { } );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    // init pipeline
    auto vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "geometryFigures_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto geomStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "geometryFigures_geom.spv" ),
      vk::ShaderStageFlagBits::eGeometry
    );
    auto fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "geometryFigures_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    vk::VertexInputBindingDescription binding( 
      0, sizeof( Vertex ), vk::VertexInputRate::eVertex
    );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0, 
        vk::Format::eR32G32B32Sfloat, offsetof( Vertex, position )
      ),
      vk::VertexInputAttributeDescription( 1, 0, 
        vk::Format::eR32G32B32A32Sfloat, offsetof( Vertex, color )
      ),
      vk::VertexInputAttributeDescription( 2, 0, 
        vk::Format::eR16Uint, offsetof( Vertex, sides )
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
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( { 
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = _device->createGraphicsPipeline( pipelineCache, { }, 
        { vertexStage, geomStage, fragmentStage }, 
      vertexInput, assembly, nullptr, viewport, rasterization, multisample, 
      depthStencil, colorBlend, dynamic, pipelineLayout, _renderPass );
  }
  void doPaint( void ) override
  {
    auto commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ),
    { vk::ClearValue( ccv ), vk::ClearValue( vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( pipeline );
    commandBuffer->bindVertexBuffer( 0, vertexBuffer, 0 );

    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    commandBuffer->draw( 4, 1, 0, 0 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent(int key, int scancode, int action, int mods)
  {
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
      switch (action)
      {
      case GLFW_PRESS:
        getWindow( )->close( );
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
};

void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Geometry 2D Figures", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      app->waitEvents( );
      app->paint( );
    }

    delete app;
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  return 0;
}