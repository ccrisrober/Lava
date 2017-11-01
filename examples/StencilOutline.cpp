#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include "utils/Camera.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

struct
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
} uboVS;

std::array<float, 1> pushConstants;

class MyApp : public VulkanApp
{
public:
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

  std::shared_ptr<Buffer> _uniformBufferMVP;
  std::shared_ptr<DescriptorSet> _descriptorSet;

  std::shared_ptr<lava::extras::Geometry> geometry;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    geometry = std::make_shared<lava::extras::Geometry>( _device, 
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "dragon.obj_" ) );

    // MVP buffer
    {
      uint32_t mvpBufferSize = sizeof( uboVS );
      _uniformBufferMVP = _device->createBuffer( mvpBufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.solid = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    vk::PushConstantRange pushConstantRange( vk::ShaderStageFlagBits::eVertex,
      0, sizeof( pushConstants ) );
    pipelineLayouts.outline = _device->createPipelineLayout( descriptorSetLayout, pushConstantRange );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );

    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "mesh_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "mesh_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, position ) ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, normal ) ),
          vk::VertexInputAttributeDescription( 2, 0, vk::Format::eR32G32Sfloat, offsetof( lava::extras::Vertex, texCoord ) )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );

    depthStencil.stencilTestEnable = true;

    // glStencilFunc( GL_ALWAYS, 1, 0xFF );
    depthStencil.back.compareOp = vk::CompareOp::eAlways;
    depthStencil.back.compareMask = 0xff;
    depthStencil.back.reference = 1;
    // glStencilMask(0xFF);
    depthStencil.back.writeMask = 0xff;
    // glEnable( GL_DEPTH );
    depthStencil.depthTestEnable = VK_TRUE;

    depthStencil.back.failOp = vk::StencilOp::eKeep;
    depthStencil.back.depthFailOp = vk::StencilOp::eKeep;
    depthStencil.back.passOp = vk::StencilOp::eReplace;

    /*depthStencil.back.failOp = vk::StencilOp::eReplace;
    depthStencil.back.depthFailOp = vk::StencilOp::eReplace;
    depthStencil.back.passOp = vk::StencilOp::eReplace;*/
    depthStencil.front = depthStencil.back;

    pipelines.solid = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.solid, _renderPass );


    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    depthStencil.back.compareOp = vk::CompareOp::eNotEqual;
    // depthStencil.back.compareMask = 0xff;
    // depthStencil.back.reference = 1;
    // glDisable( GL_DEPTH );
    depthStencil.depthTestEnable = VK_FALSE;

    /*depthStencil.back.failOp = vk::StencilOp::eKeep;
    depthStencil.back.depthFailOp = vk::StencilOp::eKeep;
    depthStencil.back.passOp = vk::StencilOp::eReplace;*/
    depthStencil.front = depthStencil.back;

    vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "outline_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "outline_frag.spv" ), vk::ShaderStageFlagBits::eFragment );


    pipelines.outline = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.outline, _renderPass );

    std::array<vk::DescriptorPoolSize, 1> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( _descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( _uniformBufferMVP, 0, sizeof( uboVS ) )
      )
    };
    _device->updateDescriptorSets( wdss, {} );

  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 3.0f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );

    uboVS.view = camera.GetViewMatrix( );
    uboVS.proj = glm::perspective( glm::radians( camera.Zoom ), ( float ) width / ( float ) height, 0.1f, 100.0f );

    uboVS.proj[ 1 ][ 1 ] *= -1;

    _uniformBufferMVP->writeData( 0, sizeof( uboVS ), &uboVS );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );

    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool =
      _device->createCommandPool(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass,
      _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 },
        _defaultFramebuffer->getExtent( ) ),
        { vk::ClearValue( ccv ), vk::ClearValue(
          vk::ClearDepthStencilValue( 1.0f, 0 ) )
        }, vk::SubpassContents::eInline );

    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    // First step: Basic rendering (fill stencil buffer)
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.solid, 0, { _descriptorSet }, nullptr );
    commandBuffer->bindGraphicsPipeline( pipelines.solid );
    geometry->render( commandBuffer );

    // Second step: Render scaled objet only where stencil was not set by first step
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.outline, 0, { _descriptorSet }, nullptr );
    commandBuffer->bindGraphicsPipeline( pipelines.outline );
    commandBuffer->pushConstants<float>( *pipelineLayouts.outline,
      vk::ShaderStageFlagBits::eVertex, 0, pushConstants );
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
  /*virtual void cursorPosEvent( double xPos, double yPos )
  {
    if ( firstMouse )
    {
      lastX = xPos;
      lastY = yPos;
      firstMouse = false;
    }

    float xoffset = xPos - lastX;
    float yoffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement( xoffset, yoffset );
  }*/
  void changeOutlineWidth( float delta )
  {
    if ( pushConstants[ 0 ] + delta > 0.01f )
    {
      pushConstants[ 0 ] += delta;
    }
  }

  virtual void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_C:
      changeOutlineWidth( -0.01f );
      break;
    case GLFW_KEY_V:
      changeOutlineWidth( 0.01f );
      break;
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
    case GLFW_KEY_W:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( FORWARD, deltaTime );
        break;
      default:
        break;
      }
      break;
    case GLFW_KEY_S:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( BACKWARD, deltaTime );
        break;
      default:
        break;
      }
      break;
    case GLFW_KEY_A:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( LEFT, deltaTime );
        break;
      default:
        break;
      }
      break;
    case GLFW_KEY_D:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( RIGHT, deltaTime );
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

int main( int argc, char** argv )
{
  try
  {
    VulkanApp* app = new MyApp( "Stencil Outline loading", 800, 600 );

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