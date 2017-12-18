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

struct
{
  glm::mat4 view;
  glm::mat4 proj;
} uboVS;

struct PushConstants
{
  glm::mat4 model;
  glm::vec3 color;
} pushConstants;

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;
};
const std::vector<Vertex> vertices =
{
  { { -0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
  { { 0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
  { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
  { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
  { { -0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
  { { -0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },

  { { -0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
  { { 0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
  { { -0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
  { { -0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },

  { { -0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
  { { -0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
  { { -0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
  { { -0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
  { { -0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
  { { -0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },

  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
  { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
  { { 0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
  { { 0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
  { { 0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },

  { { -0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
  { { 0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
  { { 0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
  { { 0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
  { { -0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
  { { -0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

  { { -0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
  { { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
  { { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
  { { -0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
  { { -0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

  { { -1.0f, -1.0f, -0.5f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
  { { 1.0f, -1.0f, -0.5f },{ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
  { { 1.0f,  1.0f, -0.5f },{ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } },
  { { 1.0f,  1.0f, -0.5f },{ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } },
  { { -1.0f,  1.0f, -0.5f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
  { { -1.0f, -1.0f, -0.5f },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } }
};

class PlanarReflectionApp : public VulkanApp
{
public:
  std::shared_ptr<VertexBuffer> vertexBuffer;
  std::shared_ptr<lava::UniformBuffer> uniformMVP;

  struct
  {
    std::shared_ptr<Pipeline> cube;
    std::shared_ptr<Pipeline> plane;
    std::shared_ptr<Pipeline> reflection;
  } pipelines;

  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<CommandPool> commandPool;
  std::shared_ptr<Texture2D> tex;

  PlanarReflectionApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex
    );
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }

    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "glass.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    uniformMVP = _device->createUniformBuffer( sizeof( uboVS ) ); 
    
    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 1,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
      _device->createDescriptorSetLayout( dslbs );

    vk::PushConstantRange pushConstantRange( vk::ShaderStageFlagBits::eVertex,
      0, sizeof( pushConstants ) );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, pushConstantRange );


    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
    };
    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( { }, 1, poolSize );

    // Init descriptor set
    descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr,
        DescriptorBufferInfo(
          uniformMVP, 0, sizeof( uboVS )
        )
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    _device->updateDescriptorSets( wdss, { } );


    // init pipeline
    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "planar_reflection_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "planar_reflection_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
          offsetof( Vertex, pos )
          ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( Vertex, color )
          ),
          vk::VertexInputAttributeDescription( 2, 0, vk::Format::eR32G32Sfloat,
            offsetof( Vertex, texCoord )
          )
        }
    );

    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencilState( { }, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );


    pipelines.cube = _device->createGraphicsPipeline( pipelineCache, { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencilState, colorBlend, 
      dynamic, pipelineLayout, _renderPass
    );

    depthStencilState.stencilTestEnable = VK_TRUE;

    // glStencilFunc( GL_ALWAYS, 1, 0xFF );
    depthStencilState.back.compareOp = vk::CompareOp::eAlways;
    depthStencilState.back.compareMask = 0xff;
    depthStencilState.back.reference = 1;
    // glStencilMask(0xFF);
    depthStencilState.back.writeMask = 0xff;
    // glDepthMask(GL_FALSE);
    depthStencilState.depthWriteEnable = VK_FALSE;

    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    depthStencilState.back.failOp = vk::StencilOp::eKeep;
    depthStencilState.back.depthFailOp = vk::StencilOp::eKeep;
    depthStencilState.back.passOp = vk::StencilOp::eReplace;

    depthStencilState.front = depthStencilState.back;

    pipelines.plane = _device->createGraphicsPipeline( pipelineCache, { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencilState, colorBlend, 
      dynamic, pipelineLayout, _renderPass
    );

    // glStencilFunc(GL_EQUAL, 1, 0xFF);
    depthStencilState.back.compareOp = vk::CompareOp::eEqual;
    // glStencilMask(0x00);
    depthStencilState.back.writeMask = 0x00;
    // glDepthMask( GL_TRUE );
    depthStencilState.depthWriteEnable = VK_TRUE;

    depthStencilState.front = depthStencilState.back;

    pipelines.reflection = _device->createGraphicsPipeline( pipelineCache, { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencilState, colorBlend, 
      dynamic, pipelineLayout, _renderPass
    );

    uboVS.view = glm::lookAt(
      glm::vec3( 2.5f, 2.5f, 2.0f ),
      glm::vec3( 0.0f, 0.0f, 0.0f ),
      glm::vec3( 0.0f, 0.0f, 1.0f )
    );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformMVP->writeData( 0, sizeof( uboVS ), &uboVS );
  }

  void doPaint( void ) override
  {
    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    auto commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), 
      vk::Rect2D( { 0, 0 }, 
      _defaultFramebuffer->getExtent( ) ),
      { vk::ClearValue( ccv ), vk::ClearValue( 
        vk::ClearDepthStencilValue( 1.0f, 0 ) )
      }, vk::SubpassContents::eInline );

    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    vertexBuffer->bind( commandBuffer );
    
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    
    vk::PipelineLayout pipl = *pipelineLayout;

    // Draw cube
    {
      pushConstants.model = glm::rotate( glm::mat4( 1.0f ),
        time * 0.5f * glm::radians( 180.0f ), glm::vec3( 0.0f, 0.0f, 1.0f )
      );
      commandBuffer->pushConstants<PushConstants>( pipl,
        vk::ShaderStageFlagBits::eVertex, 0, pushConstants
      );

      commandBuffer->bindGraphicsPipeline( pipelines.cube );
      commandBuffer->draw( 36, 1, 0, 0 );
    }

    // Draw floor
    {
      commandBuffer->bindGraphicsPipeline( pipelines.plane );
      commandBuffer->draw( 6, 1, 36, 0 );
    }

    // Draw cube reflection
    {
      pushConstants.model = glm::scale(
        glm::translate( pushConstants.model, glm::vec3( 0.0f, 0.0f, -1.0f ) ),
        glm::vec3( 1.0f, 1.0f, -1.0f )
      );
      pushConstants.color = glm::vec3( 0.3f, 0.3f, 0.3f );
      commandBuffer->pushConstants<PushConstants>( pipl,
        vk::ShaderStageFlagBits::eVertex, 0, pushConstants
      );
      commandBuffer->bindGraphicsPipeline( pipelines.reflection );
      commandBuffer->draw( 36, 1, 0, 0 );
      pushConstants.color = glm::vec3( 1.0f, 1.0f, 1.0f );
    }

    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_ESCAPE:
      switch ( action )
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

void glfwErrorCallback( int error, const char* description )
{
  fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

int main( void )
{
  try
  {
    VulkanApp* app = new PlanarReflectionApp( "Planar Reflection", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      //app->waitEvents( );
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