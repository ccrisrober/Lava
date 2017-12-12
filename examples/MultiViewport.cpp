#include <lava/lava.h>
#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include "utils/Camera.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera( glm::vec3( 0.0f, 1.0f, 6.0f ) );
// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

struct
{
  glm::mat4 model;
  glm::mat4 view[2];
  glm::mat4 proj[2];
} uboGS;

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<lava::UniformBuffer> uniformBufferMVP;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;

  std::shared_ptr<lava::extras::Geometry> geometry;

  std::shared_ptr<Pipeline> pipeline;

  std::shared_ptr<Texture2D> tex;
  std::shared_ptr<CommandPool> commandPool;

  std::shared_ptr<lava::engine::Node> node;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    node = std::make_shared<lava::engine::Node>( "GeometryNode" );
    geometry = std::make_shared<lava::extras::Geometry>( _device, 
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "wolf.obj_" ) );

    uniformBufferMVP = std::make_shared<lava::UniformBuffer>( _device, sizeof( uboGS ) );

    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "/MatCap_Toon3.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    // init pipeline
    PipelineVertexInputStateCreateInfo vertexInputBindings(
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
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState( 
      {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE
    );
    vk::PipelineRasterizationStateCreateInfo rasterizationState( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );
    vk::PipelineColorBlendAttachmentState blendAttachmentState(
      false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlendState( false, 
      vk::LogicOp::eNoOp, blendAttachmentState, { 1.0f, 1.0f, 1.0f, 1.0f }
    );

    // We use two viewports
    PipelineViewportStateCreateInfo viewportState( 2, 2 );
    
    PipelineMultisampleStateCreateInfo multisampleState(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, 
      vk::CompareOp::eAlways, 0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencilState( 
      {}, true, true, vk::CompareOp::eLessOrEqual, false, false, 
      stencilOpState, stencilOpState, 0.0f, 0.0f
    );
    PipelineDynamicStateCreateInfo dynamicStateEnables( { 
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor
    } );

    std::array<PipelineShaderStageCreateInfo, 3> shaderStages =
    {
      _device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "matcapMV_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      ),
      _device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "matcapMV_geom.spv" ),
        vk::ShaderStageFlagBits::eGeometry
      ),
      _device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "matcapMV_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      )
    };

    pipeline = _device->createGraphicsPipeline( pipelineCache, {}, shaderStages, 
      vertexInputBindings, inputAssemblyState, nullptr, viewportState, 
      rasterizationState, multisampleState,  depthStencilState, colorBlendState,
      dynamicStateEnables, pipelineLayout, _renderPass
    );

    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss = 
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr, 
        DescriptorBufferInfo( 
          uniformBufferMVP, 0, sizeof( uboGS )
        )
      ),
      WriteDescriptorSet( descriptorSet, 1, 0, 
        vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    _device->updateDescriptorSets( wdss, {} );
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( ) / 2;
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    uboGS.model = glm::mat4( 1.0f );
    uboGS.model = glm::rotate( uboGS.model, time * glm::radians( 25.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    
    uboGS.proj[0] = glm::perspective( glm::radians( camera.Zoom ), width / ( float ) height, 0.1f, 10.0f );
    uboGS.proj[ 1 ] = uboGS.proj[ 0 ];
    uboGS.proj[0][ 1 ][ 1 ] *= -1;

    camera.ProcessKeyboard( LEFT, 0.5f );
    uboGS.view[ 0 ] = camera.GetViewMatrix( );
    camera.ProcessKeyboard( LEFT, -0.5f );

    camera.ProcessKeyboard( RIGHT, 0.5f );
    uboGS.view[ 1 ] = camera.GetViewMatrix( );
    camera.ProcessKeyboard( RIGHT, -0.5f );

    uniformBufferMVP->writeData( 0, sizeof( uboGS ), &uboGS );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );

    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), 
      vk::Rect2D( { 0, 0 }, 
      _defaultFramebuffer->getExtent( ) ),
      { vk::ClearValue( ccv ), vk::ClearValue( 
        vk::ClearDepthStencilValue( 1.0f, 0 ) )
      }, vk::SubpassContents::eInline );

    commandBuffer->bindGraphicsPipeline( pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );
    std::array<vk::Viewport, 2> viewports;
    // Left
    viewports[ 0 ] = { 0, 0, ( float ) width / 2.0f, ( float ) height, 0.0, 1.0f };
    // Right
    viewports[ 1 ] = { ( float ) width / 2.0f, 0, ( float ) width / 2.0f, ( float ) height, 0.0, 1.0f };
    commandBuffer->setViewport( 0, viewports );

    std::array<vk::Rect2D, 2> scissorRects =
    {
      vk::Rect2D( vk::Offset2D( 0, 0 ), vk::Extent2D( width / 2, height ) ),
      vk::Rect2D( vk::Offset2D( width / 2, 0 ), vk::Extent2D( width / 2, height ) )
    };
    commandBuffer->setScissor( 0, scissorRects );

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
      }
      break;
    default:
      break;
    }
  }
  virtual void cursorPosEvent( double xPos, double yPos )
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
    VulkanApp* app = new MyApp( "MultiViewport", 800, 600 );

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