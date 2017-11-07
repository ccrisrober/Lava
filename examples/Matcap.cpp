#include <lava/lava.h>
using namespace lava;

#include <routes.h>

struct
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
} uboVS;

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

    uniformBufferMVP = std::make_shared<lava::UniformBuffer>( _device, sizeof( uboVS ) );

    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "/MatCap_Toon3.png" ), commandPool, _graphicsQueue,
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
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

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
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );   // one dummy viewport and scissor, as dynamic state sets them
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


    pipeline = _device->createGraphicsPipeline( pipelineCache, {}, { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _renderPass );


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
          uniformBufferMVP, 0, sizeof( uboVS )
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
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    //node->rotate( glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ); // , lava::engine::Node::TransformSpace::World );
    //uboVS.model = node->getTransform( );
    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 1.0f, 5.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformBufferMVP->writeData( 0, sizeof( uboVS ), &uboVS );
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

    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    
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
};

void glfwErrorCallback( int error, const char* description )
{
  fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Matcap", 800, 600 );

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