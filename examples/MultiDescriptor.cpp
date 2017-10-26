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
  std::shared_ptr<Buffer> _uniformBufferMVP;

  std::shared_ptr<lava::extras::Geometry> geometry;

  struct
  {
    std::shared_ptr<DescriptorSet> matcapLeft;
    std::shared_ptr<DescriptorSet> matcapCenter;
    std::shared_ptr<DescriptorSet> matcapRight;
  } descriptorSets;

  struct
  {
    std::shared_ptr<Texture2D> matcapLeft;
    std::shared_ptr<Texture2D> matcapCenter;
    std::shared_ptr<Texture2D> matcapRight;
  } textures;

  std::shared_ptr<PipelineCache> pipelineCache;
  std::shared_ptr<DescriptorPool> descriptorPool;
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;

  std::array<vk::DescriptorPoolSize, 2> poolSize;

  void createMaterial( std::shared_ptr<CommandPool>& commandPool, 
    const std::string& texPath,
    std::shared_ptr<DescriptorSetLayout>& descriptorSetLayout,
    std::shared_ptr<DescriptorSet>& descriptorSet,
    std::shared_ptr<Texture2D>& texture )
  {
    texture = std::make_shared<Texture2D>( _device, texPath,
      commandPool, _graphicsQueue, vk::Format::eR8G8B8A8Unorm );

    // Init descriptor set
    descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          _uniformBufferMVP, 0, sizeof( uboVS )
        )
      ),
      WriteDescriptorSet( descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        texture->descriptor, nullptr
      )
    };
    _device->updateDescriptorSets( wdss, {} );
  }

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    geometry = std::make_shared<lava::extras::Geometry>( _device, 
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "wolf.obj_" ) );

    // MVP buffer
    {
      uint32_t mvpBufferSize = sizeof(uboVS);
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
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

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
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, position ) ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, normal ) )
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


    pipeline = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _renderPass
    );

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    // init pipeline
    pipelineCache = _device->createPipelineCache( 0, nullptr );

    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 3 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 3 );
    descriptorPool = _device->createDescriptorPool( {}, 3, poolSize );


    createMaterial( commandPool,
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/MatCap_Toon3.png" ),
      descriptorSetLayout,
      descriptorSets.matcapLeft, textures.matcapLeft
    );

    createMaterial( commandPool,
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/green_matcap.jpg" ),
      descriptorSetLayout,
      descriptorSets.matcapCenter, textures.matcapCenter
    );

    createMaterial( commandPool,
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/rubymatcap.jpg" ),
      descriptorSetLayout,
      descriptorSets.matcapRight, textures.matcapRight
    );

    std::cout << "Pipeline created!" << std::endl;
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 1.0f, 5.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    _uniformBufferMVP->writeData( 0, sizeof( uboVS ), &uboVS );
  }

  bool enable_wire = false;

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

    uint32_t width = _defaultFramebuffer->getExtent( ).width;
    uint32_t height = _defaultFramebuffer->getExtent( ).height;

    vk::Rect2D scissor( { 0, 0 }, { width, height } );

    commandBuffer->setViewport( 0, vk::Viewport( 0.0f, 0.0f, width, height ) );

    commandBuffer->bindGraphicsPipeline( pipeline );

    // Left pipeline
    scissor.offset.x = 0;
    scissor.extent.width = ( float ) width / 3.0;
    commandBuffer->setScissor( 0, scissor );

    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSets.matcapLeft }, nullptr );

    geometry->render( commandBuffer );

    // Center pipeline
    scissor.offset.x = ( float ) width / 3.0;
    commandBuffer->setScissor( 0, scissor );

    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSets.matcapCenter }, nullptr );

    geometry->render( commandBuffer );

    // Right pipeline
    scissor.offset.x = ( float ) width / 3.0 + ( float ) width / 3.0;
    commandBuffer->setScissor( 0, scissor );

    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSets.matcapRight }, nullptr );

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
    VulkanApp* app = new MyApp( "MultiPipeline", 800, 600 );

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