#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <routes.h>

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 proj;
} uboVS;

struct Vertex
{
  glm::vec3 pos;
};

const float side = 5.0f;
const float side2 = side / 2.0f;
const std::vector<Vertex> vertices =
{
  { { -side2, -side2,  side2 } },
  { { side2, -side2,  side2 } },
  { { -side2,  side2,  side2 } },
  { { side2,  side2,  side2 } },

  { { -side2, -side2, -side2 } },
  { { side2, -side2, -side2 } },
  { { -side2,  side2, -side2 } },
  { { side2,  side2, -side2 } },

  { { side2, -side2, -side2 } },
  { { side2, -side2,  side2 } },
  { { side2,  side2, -side2 } },
  { { side2,  side2,  side2 } },

  { { -side2, -side2, -side2 } },
  { { -side2, -side2,  side2 } },
  { { -side2,  side2, -side2 } },
  { { -side2,  side2,  side2 } },

  { { -side2,  side2, -side2 } },
  { { -side2,  side2,  side2 } },
  { { side2,  side2, -side2 } },
  { { side2,  side2,  side2 } },

  { { -side2, -side2, -side2 } },
  { { -side2, -side2,  side2 } },
  { { side2, -side2, -side2 } },
  { { side2, -side2,  side2 } }
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
  std::shared_ptr<Buffer> _uniformBufferMVP;
  std::shared_ptr<Pipeline> _pipeline;
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<DescriptorSet> _descriptorSet;
  std::shared_ptr<TextureCubemap> tex;

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

    // MVP buffer
    {
      uint32_t mvpBufferSize = sizeof( UniformBufferObject );
      _uniformBufferMVP = _device->createBuffer( mvpBufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::array< std::string, 6 > cubeImages =
    {
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/cubemap/right.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/cubemap/left.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/cubemap/top.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/cubemap/bottom.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/cubemap/back.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/cubemap/front.jpg" ),
    };
    tex = std::make_shared<TextureCubemap>( _device, cubeImages, commandPool, 
      _graphicsQueue, vk::Format::eR8G8B8A8Unorm );


    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    DescriptorSetLayoutBinding mvpDescriptor( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex );
    dslbs.push_back( mvpDescriptor );
    DescriptorSetLayoutBinding mvpDescriptor2( 1, vk::DescriptorType::eCombinedImageSampler,
      vk::ShaderStageFlagBits::eFragment );
    dslbs.push_back( mvpDescriptor2 );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );



    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule =
      _device->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "/skybox_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    std::shared_ptr<ShaderModule> fragmentShaderModule =
      _device->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "/skybox_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
    PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );
    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos ) )
    } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
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


    _pipeline = _device->createGraphicsPipeline( pipelineCache, {}, { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, _renderPass );

    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;

    WriteDescriptorSet w( _descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( _uniformBufferMVP, 0, sizeof( UniformBufferObject ) ) );
    wdss.push_back( w );

    WriteDescriptorSet w2( _descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
      DescriptorImageInfo(
        vk::ImageLayout::eGeneral,
        std::make_shared<vk::ImageView>( tex->view ),
        std::make_shared<vk::Sampler>( tex->sampler )
      ), nullptr
    );
    wdss.push_back( w2 );
    _device->updateDescriptorSets( wdss, {} );
  }
  glm::vec3 rotation = { -7.25f, -120.0f, 0.0f };
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.proj = glm::perspective( glm::radians( 60.0f ), ( float ) width / ( float ) height, 0.001f, 256.0f );

    rotation.y = 0.025 * time;

    uboVS.model = glm::mat4( );
    uboVS.model = glm::translate( uboVS.model, glm::vec3( 0, 0, 0 ) );
    uboVS.model = glm::rotate( uboVS.model, glm::radians( rotation.x ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
    uboVS.model = glm::rotate( uboVS.model, glm::radians( rotation.y ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    uboVS.model = glm::rotate( uboVS.model, glm::radians( rotation.z ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

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
    //if (glfwInit())
    //{
    VulkanApp* app = new MyApp( "Skybox", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      app->waitEvents( );
      app->paint( );
    }

    delete app;
    //}
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  // system( "PAUSE" );
  return 0;
}