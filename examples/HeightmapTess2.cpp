#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include "utils/Camera.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera( glm::vec3( 0.0f, 1.0f, 3.5f ) );
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

struct Vertex
{
  glm::vec3 pos;
  glm::vec2 texCoord;
};

const float side = 1.0f;
const float side2 = side / 2.0f;
std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<VertexBuffer> _vertexBuffer;
  std::shared_ptr<IndexBuffer> _indexBuffer;
  std::shared_ptr<Buffer> _uniformBufferMVP;

  struct Pipelines
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> wireframe;
  } pipelines;

  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<DescriptorSet> _descriptorSet;
  std::shared_ptr<Texture2D> texHeightmap;

  void generatePlane( float width = 1.0f, float height = 1.0f,
    unsigned int gridX = 1,
    unsigned int gridY = 1 )
  {
    float width_half = width / 2.0f;
    float height_half = height / 2.0f;

    unsigned int gridX1 = gridX + 1;
    unsigned int gridY1 = gridY + 1;

    float segment_width = width / gridX;
    float segment_height = height / gridY;

    unsigned int ix, iy;

    for ( iy = 0; iy < gridY1; ++iy )
    {
      float y = iy * segment_height - height_half;
      for ( ix = 0; ix < gridX1; ++ix )
      {
        float x = ix * segment_width - width_half;

        vertices.push_back( Vertex
        {
          glm::vec3( x, 0.0f, -y ),
          glm::vec2( ( ( float ) ix ) / gridX, 1.0 - ( ( ( float ) iy ) / gridY ) )
        } );
      }
    }
    for ( iy = 0; iy < gridY; ++iy )
    {
      for ( ix = 0; ix < gridX; ++ix )
      {
        unsigned int a = ix + gridX1 * iy;
        unsigned int b = ix + gridX1 * ( iy + 1 );
        unsigned int c = ( ix + 1 ) + gridX1 * ( iy + 1 );
        unsigned int d = ( ix + 1 ) + gridX1 * iy;

        // faces
        indices.push_back( a );   indices.push_back( b );    indices.push_back( d );
        indices.push_back( b );   indices.push_back( c );    indices.push_back( d );
      }
    }

  }

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    generatePlane( 2.5f, 2.5f, 25, 25 );

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
        vk::IndexType::eUint32, indices.size( ) );
      _indexBuffer->writeData( 0, indexBufferSize, indices.data( ) );
    }

    // MVP buffer
    {
      uint32_t mvpBufferSize = sizeof( uboVS );
      _uniformBufferMVP = _device->createBuffer( mvpBufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    texHeightmap = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "heightmap.jpg" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );



    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs = 
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | 
        vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eTessellationControl
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eTessellationEvaluation | 
        vk::ShaderStageFlagBits::eTessellationControl | vk::ShaderStageFlagBits::eFragment
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );



    // init shaders

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage =
      _device->createShaderPipelineShaderStage( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "terrain2_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    PipelineShaderStageCreateInfo geomStage =
      _device->createShaderPipelineShaderStage( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "terrain2_geom.spv" ), vk::ShaderStageFlagBits::eGeometry );
    PipelineShaderStageCreateInfo ctrlStage =
      _device->createShaderPipelineShaderStage( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "terrain2_tesc.spv" ), vk::ShaderStageFlagBits::eTessellationControl );
    PipelineShaderStageCreateInfo evalStage =
      _device->createShaderPipelineShaderStage( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "terrain2_tese.spv" ), vk::ShaderStageFlagBits::eTessellationEvaluation );
    PipelineShaderStageCreateInfo fragmentStage =
      _device->createShaderPipelineShaderStage( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "terrain2_frag.spv" ), vk::ShaderStageFlagBits::eFragment );
    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos ) ),
      vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord ) ) }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::ePatchList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );   // one dummy viewport and scissor, as dynamic state sets them
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );

    vk::PipelineTessellationStateCreateInfo tessState( { }, 3 );

    pipelines.solid = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage, geomStage, fragmentStage, ctrlStage, evalStage },
      vertexInput, assembly, tessState, viewport, rasterization, multisample,
      depthStencil, colorBlend, dynamic, _pipelineLayout, _renderPass );

    // Wireframe rendering pipeline
    if ( _physicalDevice->getDeviceFeatures( ).fillModeNonSolid )
    {
      rasterization.polygonMode = vk::PolygonMode::eLine;
      rasterization.lineWidth = 1.0f;

      rasterization.cullMode = vk::CullModeFlagBits::eNone;

      pipelines.wireframe = _device->createGraphicsPipeline( pipelineCache, {},
      { vertexStage, geomStage, fragmentStage, ctrlStage, evalStage },
        vertexInput, assembly, tessState, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, _pipelineLayout, _renderPass );
    }

    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( _descriptorSet, 0, 0, 
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( _uniformBufferMVP, 0, sizeof( uboVS ) )
      ),
      WriteDescriptorSet( _descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        texHeightmap->descriptor, nullptr
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

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    uboVS.model = glm::mat4( 1.0f );
    //uboVS.model = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) );
    //uboVS.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
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


    if ( enable_wire )
    {
      std::cout << "WIREFRAME PIPELINE" << std::endl;
      commandBuffer->bindGraphicsPipeline( pipelines.wireframe );
    }
    else
    {
      std::cout << "SOLID PIPELINE" << std::endl;
      commandBuffer->bindGraphicsPipeline( pipelines.solid );
    }


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
  bool enable_wire = true;
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_W:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( FORWARD, deltaTime );
        break;
      }
      break;
    case GLFW_KEY_S:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( BACKWARD, deltaTime );
        break;
      }
      break;
    case GLFW_KEY_A:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( LEFT, deltaTime );
        break;
      }
      break;
    case GLFW_KEY_D:
      switch ( action )
      {
      case GLFW_PRESS:
        camera.ProcessKeyboard( RIGHT, deltaTime );
        break;
      }
      break;
    case GLFW_KEY_E:
      enable_wire = true;
      break;
    case GLFW_KEY_R:
      enable_wire = false;
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
    VulkanApp* app = new MyApp( "Terrain Heightmap (Tesselation)", SCR_WIDTH, SCR_HEIGHT );

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