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
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
} uboVS;

struct
{
  float outlineWidth = 0.05f;
} uboVS2;

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<lava::UniformBuffer> uniformMVP;
  std::shared_ptr<lava::UniformBuffer> uniformBufferOutline;

  struct
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> outline;
  } pipelines;

  struct
  {
    std::shared_ptr<PipelineLayout> solid;
    std::shared_ptr<PipelineLayout> outline;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr<DescriptorSet> solid;
    std::shared_ptr<DescriptorSet> outline;
  } descriptorSets;

  struct
  {
    std::shared_ptr<DescriptorSetLayout> solid;
    std::shared_ptr<DescriptorSetLayout> outline;
  } descriptorSetLayouts;

  std::shared_ptr<lava::extras::Geometry> geometry;

  std::shared_ptr<Texture2D> tex;
  std::shared_ptr<CommandPool> commandPool;

  std::shared_ptr<lava::engine::Node> node;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    node = std::make_shared<lava::engine::Node>( "GeometryNode" );
    geometry = std::make_shared<lava::extras::Geometry>( _device, 
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "wolf.obj_" ) );

    uniformMVP = std::make_shared<lava::UniformBuffer>( 
      _device, sizeof( uboVS )
    );
    uniformBufferOutline = std::make_shared<lava::UniformBuffer>( 
      _device, sizeof( uboVS2 )
    );

    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "/green_matcap.jpg" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };
    descriptorSetLayouts.solid = _device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.solid = _device->createPipelineLayout( descriptorSetLayouts.solid, nullptr );

    dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };
    descriptorSetLayouts.outline = _device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.outline = _device->createPipelineLayout( descriptorSetLayouts.outline, nullptr );

    // init pipeline
    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "matcap_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "matcap_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof( lava::extras::Vertex, position )
          ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof( lava::extras::Vertex, normal )
          )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencilState( { }, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );

    depthStencilState.stencilTestEnable = VK_TRUE;

    // glStencilFunc( GL_ALWAYS, 1, 0xFF );
    depthStencilState.back.compareOp = vk::CompareOp::eAlways;
    depthStencilState.back.compareMask = 0xff;
    depthStencilState.back.reference = 1;
    // glStencilMask(0xFF);
    depthStencilState.back.writeMask = 0xff;
    // glEnable( GL_DEPTH );
    depthStencilState.depthTestEnable = VK_TRUE;

    depthStencilState.back.failOp = vk::StencilOp::eKeep;
    depthStencilState.back.depthFailOp = vk::StencilOp::eKeep;
    depthStencilState.back.passOp = vk::StencilOp::eReplace;

    //depthStencilState.back.failOp = vk::StencilOp::eReplace;
    //depthStencilState.back.depthFailOp = vk::StencilOp::eReplace;
    //depthStencilState.back.passOp = vk::StencilOp::eReplace;
    depthStencilState.front = depthStencilState.back;

    pipelines.solid = _device->createGraphicsPipeline( pipelineCache, { }, 
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencilState, colorBlend, 
      dynamic, pipelineLayouts.solid, _renderPass
    );

    // Outline pass
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    depthStencilState.back.compareOp = vk::CompareOp::eNotEqual;
    // depthStencilState.back.compareMask = 0xff;
    // depthStencilState.back.reference = 1;
    // glDisable( GL_DEPTH );
    depthStencilState.depthTestEnable = VK_FALSE;

    //depthStencilState.back.failOp = vk::StencilOp::eKeep;
    //depthStencilState.back.depthFailOp = vk::StencilOp::eKeep;
    //depthStencilState.back.passOp = vk::StencilOp::eReplace;
    depthStencilState.front = depthStencilState.back;

    vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "outline_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "outline_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    pipelines.outline = _device->createGraphicsPipeline( pipelineCache, { },
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencilState, colorBlend,
      dynamic, pipelineLayouts.outline, _renderPass
    );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 3 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
    };
    auto descriptorPool = _device->createDescriptorPool( { }, 2, poolSize );

    // Init descriptor set
    descriptorSets.solid = _device->allocateDescriptorSet( 
      descriptorPool, descriptorSetLayouts.solid
    );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSets.solid, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo(
        uniformMVP, 0, sizeof( uboVS )
      )
      ),
      WriteDescriptorSet( descriptorSets.solid, 1, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    _device->updateDescriptorSets( wdss, { } );

    descriptorSets.outline = _device->allocateDescriptorSet( 
      descriptorPool, descriptorSetLayouts.outline
    );
    wdss =
    {
      WriteDescriptorSet( descriptorSets.outline, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uniformMVP, 0, sizeof( uboVS )
        )
      ),
      WriteDescriptorSet( descriptorSets.outline, 1, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uniformBufferOutline, 0, sizeof( uboVS2 )
        )
      )
    };
    _device->updateDescriptorSets( wdss, { } );
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    //node->rotate( glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ); // , lava::engine::Node::TransformSpace::World );
    //uboVS.model = node->getTransform( );
    uboVS.model = glm::mat4( 1.0f );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 90.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) );
    //uboVS.model = glm::rotate( uboVS.model, glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 1.0f, 5.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformMVP->writeData( 0, sizeof( uboVS ), &uboVS );

    uniformBufferOutline->writeData( 0, sizeof( uboVS2 ), &uboVS2 );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );

    auto commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 0.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), 
      vk::Rect2D( { 0, 0 }, 
      _defaultFramebuffer->getExtent( ) ),
      { vk::ClearValue( ccv ), vk::ClearValue( 
        vk::ClearDepthStencilValue( 1.0f, 0 ) )
      }, vk::SubpassContents::eInline );

    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    // First pass renders object (toon shaded) and fills stencil buffer
    commandBuffer->bindGraphicsPipeline( pipelines.solid );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.solid, 0, { descriptorSets.solid }, nullptr );
      
    geometry->render( commandBuffer );

    // Second pass renders scaled object only where stencil was not set by first pass
    commandBuffer->bindGraphicsPipeline( pipelines.outline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.outline, 0, { descriptorSets.outline }, nullptr );

    geometry->render( commandBuffer );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void updateOutlineWidth( float inc )
  {
    if ( uboVS2.outlineWidth + inc > 0.01f )
    {
      uboVS2.outlineWidth += inc;
    }
  }
  void keyEvent( int key, int, int action, int )
  {
    switch ( key )
    {
    case GLFW_KEY_Z:
      updateOutlineWidth( -0.005f );
      break;
    case GLFW_KEY_X:
      updateOutlineWidth( 0.005f );
      break;
    case GLFW_KEY_ESCAPE:
      switch ( action )
      {
      case GLFW_PRESS:
        getWindow( )->close( );
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
    VulkanApp* app = new MyApp( "Stencil Outline", 800, 600 );

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