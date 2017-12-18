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
  glm::vec3 Kd = glm::vec3( 0.0f, 0.82f, 0.039f );
  float Sigma = 150.0f;
  VkBool32 TronEffect = false;
} uboPP;

class FresnelApp : public VulkanApp
{
public:
  struct
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> postprocess;
  } pipelines;

  struct
  {
    std::shared_ptr<PipelineLayout> solid;
    std::shared_ptr<PipelineLayout> postprocess;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr<DescriptorSet> solid;
    std::shared_ptr<DescriptorSet> postprocess;
  } descriptorSets;

  std::shared_ptr<Buffer> uniformMVP;
  std::shared_ptr<Buffer> uniformPP;

  struct
  {
    std::shared_ptr<DescriptorSetLayout> solid;
    std::shared_ptr<DescriptorSetLayout> postprocess;
  } descriptorSetLayouts;

  std::shared_ptr<lava::extras::Geometry> geometry;

  std::shared_ptr<lava::extras::CustomFBO> fbo;

  std::shared_ptr<CommandPool> commandPool;

  FresnelApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    geometry = std::make_shared<lava::extras::Geometry>( _device,
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "dragon.obj_" ) );

    fbo = std::make_shared<lava::extras::CustomFBO>( _device, width, height );

    fbo->addColorAttachmentt( vk::Format::eR16G16Sfloat );
    //fbo->addDepthAttachment( this->_depthFormat );

    fbo->build( );

    // MVP buffer
    {
      uint32_t bufferSize = sizeof( uboVS );
      uniformMVP = _device->createBuffer( bufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      // Binding 0 : Vertex shader uniform buffer
      DescriptorSetLayoutBinding(
        0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };
    descriptorSetLayouts.solid = _device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.solid = _device->createPipelineLayout( descriptorSetLayouts.solid );
    dslbs =
    {
      DescriptorSetLayoutBinding(
        0,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding(
        1,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eFragment
      )
    };
    descriptorSetLayouts.postprocess = _device->createDescriptorSetLayout( dslbs );
    pipelineLayouts.postprocess = _device->createPipelineLayout( descriptorSetLayouts.postprocess );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0 );

    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "depthShader_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "depthShader_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0,
          vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, position ) ),
          vk::VertexInputAttributeDescription( 1, 0,
          vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, normal ) )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, false, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( true,
      vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 0.0f, 0.0f, 0.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport,
      vk::DynamicState::eScissor } );


    pipelines.solid = _device->createGraphicsPipeline( pipelineCache, { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.solid, fbo->renderPass );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 3 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
    };
    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( { }, 2, poolSize );

    // Init descriptor set
    descriptorSets.solid = _device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayouts.solid );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSets.solid, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformMVP, 0,
          sizeof( uboVS ) )
      )
    };
    _device->updateDescriptorSets( wdss, { } );


    // POSTPROCESS PIPELINE
    {
      uint32_t bufferSize = sizeof( uboPP );
      uniformPP = _device->createBuffer( bufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
    }
    // Init descriptor set
    descriptorSets.postprocess = _device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayouts.postprocess );

    wdss =
    {
      WriteDescriptorSet(
        descriptorSets.postprocess, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        DescriptorImageInfo(
          vk::ImageLayout::eShaderReadOnlyOptimal,
          std::make_shared<vk::ImageView>( *fbo->_colorAttachments[ 0 ].view ),
          std::make_shared<vk::Sampler>( fbo->colorSampler )
        ), nullptr
      ),
      WriteDescriptorSet(
        descriptorSets.postprocess, 1, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformPP, 0,
          sizeof( uboPP ) )
      )
    };
    _device->updateDescriptorSets( wdss, { } );

    PipelineVertexInputStateCreateInfo emptyInputState( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assemblyPP( { },
      vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );

    PipelineShaderStageCreateInfo ppVertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    PipelineShaderStageCreateInfo ppFragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "absorptionShader_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    colorBlendAttachment = vk::PipelineColorBlendAttachmentState( false,
      vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
      vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    colorBlend = PipelineColorBlendStateCreateInfo( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );

    pipelines.postprocess = _device->createGraphicsPipeline( pipelineCache, { },
    { ppVertexStage, ppFragmentStage }, emptyInputState, assemblyPP, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.postprocess, _renderPass );
    buildCmdBuffers( );
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 2.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );

    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformMVP->writeData( 0, sizeof( uboVS ), &uboVS );
    uniformPP->writeData( 0, sizeof( uboPP ), &uboPP );
  }

  std::shared_ptr<CommandBuffer> cmdSolidBuffer;

  void buildCmdBuffers( void )
  {
    uint32_t width = _defaultFramebuffer->getExtent( ).width;
    uint32_t height = _defaultFramebuffer->getExtent( ).height;

    cmdSolidBuffer = commandPool->allocateCommandBuffer( );

    cmdSolidBuffer->begin( );

    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    cmdSolidBuffer->beginRenderPass( fbo->renderPass,
      fbo->_fbo,
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        {
          vk::ClearValue( ccv ),
          //vk::ClearValue( vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline );

    cmdSolidBuffer->setViewportScissors( width, height );
    cmdSolidBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.solid, 0, { descriptorSets.solid }, nullptr );
    cmdSolidBuffer->bindGraphicsPipeline( pipelines.solid );
    geometry->render( cmdSolidBuffer );
    cmdSolidBuffer->endRenderPass( );

    cmdSolidBuffer->end( );


    fbo->commandBuffer = commandPool->allocateCommandBuffer( );

    fbo->commandBuffer->begin( );

    ccv = { 1.0f, 1.0f, 1.0f, 1.0f };
    fbo->commandBuffer->beginRenderPass( _renderPass,
      _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline );

    fbo->commandBuffer->setViewportScissors( width, height );
    fbo->commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.postprocess, 0, { descriptorSets.postprocess }, nullptr );
    fbo->commandBuffer->bindGraphicsPipeline( pipelines.postprocess );
    fbo->commandBuffer->draw( 4, 1, 0, 0 );
    fbo->commandBuffer->endRenderPass( );

    fbo->commandBuffer->end( );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );
    buildCmdBuffers( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) }, // wait 
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      cmdSolidBuffer,
      { fbo->semaphore }// signal
    } );

    _graphicsQueue->submit( SubmitInfo{
      { fbo->semaphore },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      fbo->commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_Z:
      uboPP.TronEffect = false;
      break;
    case GLFW_KEY_X:
      uboPP.TronEffect = true;
      break;
    case GLFW_KEY_ESCAPE:
      getWindow( )->close( );
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

int main( int argc, char** argv )
{
  try
  {
    VulkanApp* app = new FresnelApp( "Fresnel", 800, 600 );

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
  //system( "PAUSE" );
  return 0;
}