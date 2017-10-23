#include <lava/lava.h>
using namespace lava;

#include <routes.h>

struct
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
} uboVS;

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 texCoord;
};

const float side = 1.0f;
const float side2 = side / 2.0f;
const std::vector<Vertex> vertices =
{
  {{-side2, -side2,  side2},{ 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f}},
  {{ side2, -side2,  side2},{ 0.0f, 0.0f, 1.0f }, {1.0f, 0.0f}},
  {{-side2,  side2,  side2},{ 0.0f, 0.0f, 1.0f }, {0.0f, 1.0f}},
  {{ side2,  side2,  side2},{ 0.0f, 0.0f, 1.0f }, {1.0f, 1.0f}},

  {{-side2, -side2, -side2},{ 1.0f, 0.0f, 0.0f }, {0.0f, 0.0f}},
  {{ side2, -side2, -side2},{ 1.0f, 0.0f, 0.0f }, {1.0f, 0.0f}},
  {{-side2,  side2, -side2},{ 1.0f, 0.0f, 0.0f }, {0.0f, 1.0f}},
  {{ side2,  side2, -side2},{ 1.0f, 0.0f, 0.0f }, {1.0f, 1.0f}},

  {{ side2, -side2, -side2},{ 0.0f, 0.0f, -1.0f }, {0.0f, 0.0f}},
  {{ side2, -side2,  side2},{ 0.0f, 0.0f, -1.0f }, {1.0f, 0.0f}},
  {{ side2,  side2, -side2},{ 0.0f, 0.0f, -1.0f }, {0.0f, 1.0f}},
  {{ side2,  side2,  side2},{ 0.0f, 0.0f, -1.0f }, {1.0f, 1.0f}},

  {{-side2, -side2, -side2},{ -1.0f, 0.0f, 0.0f }, {0.0f, 0.0f}},
  {{-side2, -side2,  side2},{ -1.0f, 0.0f, 0.0f }, {1.0f, 0.0f}},
  {{-side2,  side2, -side2},{ -1.0f, 0.0f, 0.0f }, {0.0f, 1.0f}},
  {{-side2,  side2,  side2},{ -1.0f, 0.0f, 0.0f }, {1.0f, 1.0f}},

  {{-side2,  side2, -side2},{ 0.0f, -1.0f, 0.0f }, {0.0f, 0.0f}},
  {{-side2,  side2,  side2},{ 0.0f, -1.0f, 0.0f }, {1.0f, 0.0f}},
  {{ side2,  side2, -side2},{ 0.0f, -1.0f, 0.0f }, {0.0f, 1.0f}},
  {{ side2,  side2,  side2},{ 0.0f, -1.0f, 0.0f }, {1.0f, 1.0f}},

  {{-side2, -side2, -side2},{ 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f}},
  {{-side2, -side2,  side2},{ 0.0f, 1.0f, 0.0f }, {1.0f, 0.0f}},
  {{ side2, -side2, -side2},{ 0.0f, 1.0f, 0.0f }, {0.0f, 1.0f}},
  {{ side2, -side2,  side2},{ 0.0f, 1.0f, 0.0f }, {1.0f, 1.0f}}
};
const std::vector<uint16_t> indices =
{
  0,1,2,      1,3,2,
  4,6,5,      5,6,7,
  8,10,9,     9,10,11,
  12,13,14,   13,15,14,
  16,17,18,   17,19,18,
  20,22,21,   21,22,23,
};

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<VertexBuffer> _vertexBuffer;
  std::shared_ptr<IndexBuffer> _indexBuffer;
  std::shared_ptr<Pipeline> _pipeline;
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<DescriptorSet> _descriptorSet;
  std::shared_ptr<Texture2D> tex;

  std::shared_ptr<Texture2D> colorMap;
  std::shared_ptr<Texture2D> normalMap;

  struct
  {
    std::shared_ptr<Buffer> vertexShader;
    std::shared_ptr<Buffer> fragmentShader;
  } uniformBuffers;

  struct
  {
    struct
    {
      glm::mat4 proj;
      glm::mat4 view;
      glm::mat4 model;
      glm::vec4 lightPos = glm::vec4( 0.0f, -2.0f, 0.0f, 1.0f );
      glm::vec4 cameraPos;
    } vertexShader;

    struct
    {
      float heightScale = 0.1f;
      // Basic parallax mapping needs a bias to look any good (and is hard to tweak)
      float parallaxBias = -0.02f;
      // Number of layers for steep parallax and parallax occlusion (more layer = better result for less performance)
      float numLayers = 48.0f;
      // (Parallax) mapping mode to use
      int32_t mappingMode = 4;
    } fragmentShader;

  } ubos;

  float timer = 0.5f;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      _vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      _vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );
      _indexBuffer = std::make_shared<IndexBuffer>( _device, 
        vk::IndexType::eUint16, indices.size( ) );
      _indexBuffer->writeData( 0, indexBufferSize, indices.data( ) );
    }

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    // LOAD ASSETS
    colorMap = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "chesterfieldDiffuseMap.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );
    normalMap = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "chesterfieldNormalMap.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    // UNIFORM BUFFERS
    uniformBuffers.vertexShader = _device->createBuffer( sizeof( ubos.vertexShader ),
      vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive, 
      nullptr, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
    uniformBuffers.fragmentShader = _device->createBuffer( sizeof( ubos.fragmentShader ),
      vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive,
      nullptr, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );


    // UPDATE UNIFORMS
    updateUniforms( );


    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    dslbs.push_back(
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment ) );
    dslbs.push_back(
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment ) );
    dslbs.push_back(
      DescriptorSetLayoutBinding( 2, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment ) );
    dslbs.push_back(
      DescriptorSetLayoutBinding( 3, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eFragment ) );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
      _device->createDescriptorSetLayout( dslbs );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );


    // SETUP DESCRIPTOR POOL
    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 2, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss;
    wdss.push_back( WriteDescriptorSet( _descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( uniformBuffers.vertexShader, 0, 
        sizeof( ubos.vertexShader ) ) ) );

    wdss.push_back( WriteDescriptorSet( _descriptorSet, 1, 0,
      vk::DescriptorType::eCombinedImageSampler, 1,
      colorMap->descriptor, nullptr ) );

    wdss.push_back( WriteDescriptorSet( _descriptorSet, 2, 0,
      vk::DescriptorType::eCombinedImageSampler, 1,
      normalMap->descriptor, nullptr ) );

    wdss.push_back( WriteDescriptorSet( _descriptorSet, 3, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( uniformBuffers.fragmentShader, 0, 
        sizeof( ubos.fragmentShader ) ) ) );

    _device->updateDescriptorSets( wdss, {} );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "normal_mapping_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "normal_mapping_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo entryInput( 
      vk::VertexInputBindingDescription( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex ), {
        vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos) ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, normal ) ),
        vk::VertexInputAttributeDescription( 2, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord ) )
      }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } ); // Dynamic viewport and scissor
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


    _pipeline = _device->createGraphicsPipeline( pipelineCache, {}, 
      { vertexStage, fragmentStage },
      entryInput, assembly, nullptr, viewport, rasterization, multisample,
      depthStencil, colorBlend, dynamic, _pipelineLayout, _renderPass
    );
  }
  void updateUniforms( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    // Vertex shader
    ubos.vertexShader.model = glm::mat4( ); //glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubos.vertexShader.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubos.vertexShader.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    ubos.vertexShader.proj[ 1 ][ 1 ] *= -1;

    // if ( !paused ) {
    ubos.vertexShader.lightPos.x = sin( glm::radians( time * 360.0f ) ) * 1.5f;
    ubos.vertexShader.lightPos.z = cos( glm::radians( time * 360.0f ) ) * 1.5f;
    //}

    ubos.vertexShader.cameraPos = glm::vec4( glm::vec3( 0.0f, 1.25f, 1.5f ), -1.0f ) * -1.0f;

    uniformBuffers.vertexShader->writeData( 0, sizeof( ubos.vertexShader ), &ubos.vertexShader );
    uniformBuffers.fragmentShader->writeData( 0, sizeof( ubos.fragmentShader ), &ubos.fragmentShader );
  }
  void doPaint( void ) override
  {
    updateUniforms( );
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
    commandBuffer->bindGraphicsPipeline( _pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      _pipelineLayout, 0, { _descriptorSet }, nullptr );
    _vertexBuffer->bind( commandBuffer );
    _indexBuffer->bind( commandBuffer );
    
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    commandBuffer->drawIndexed( indices.size( ), 1, 0, 0, 1 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void toggleMappingMode( void )
  {
    ++ubos.fragmentShader.mappingMode;
    if ( ubos.fragmentShader.mappingMode > 4 )
    {
      ubos.fragmentShader.mappingMode = 0;
    };
    updateUniforms( );
  }
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_SPACE:
      switch ( action )
      {
      case GLFW_PRESS:
        toggleMappingMode( );
        break;
        break;
      default:
        break;
      }
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
    VulkanApp* app = new MyApp( "Normal Mapping", 800, 600 );

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