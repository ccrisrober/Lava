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

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<Texture2D> tex;
  std::shared_ptr<CommandPool> commandPool;

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "uv_checker.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 
        0, vk::DescriptorType::eCombinedImageSampler, 
        vk::ShaderStageFlagBits::eFragment
      )
    };
    auto descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( { }, 1, {
        { vk::DescriptorType::eCombinedImageSampler, 1 }
      } );

    // Init descriptor set
    descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss = 
    {
      WriteDescriptorSet( descriptorSet, 0, 0, 
        vk::DescriptorType::eCombinedImageSampler, 1, 
        tex->descriptor, nullptr
      )
    };

    _device->updateDescriptorSets( wdss, { } );

    // init pipeline
    auto vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, 
      vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, false, false, 
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, 
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( 
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, 
      stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG 
      | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, 
      vk::DynamicState::eScissor } );

    pipeline = _device->createGraphicsPipeline( pipelineCache, { }, 
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _renderPass );
  }
  void doPaint( void ) override
  {
    auto commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 }, 
        _defaultFramebuffer->getExtent( ) ),
    { vk::ClearValue( ccv ), vk::ClearValue( 
      vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

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
    VulkanApp* app = new MyApp( "FullQuadTexture", 800, 600 );

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