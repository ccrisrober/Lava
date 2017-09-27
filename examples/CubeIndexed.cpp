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

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
};

const float side = 1.0f;
const float side2 = side / 2.0f;
const std::vector<Vertex> vertices =
{
  { { -side2, -side2, side2 },{ 0.0f, 0.0f, 1.0f } },
  { { side2, -side2, side2 },{ 0.0f, 0.0f, 1.0f } },
  { { side2, side2, side2 },{ 0.0f, 0.0f, 1.0f } },
  { { -side2, side2, side2 },{ 0.0f, 0.0f, 1.0f } },


  { { side2, -side2, side2 },{ 1.0f, 0.0f, 0.0f } },
  { { side2, -side2, -side2 },{ 1.0f, 0.0f, 0.0f } },
  { { side2, side2, -side2 },{ 1.0f, 0.0f, 0.0f } },
  { { side2, side2, side2 },{ 1.0f, 0.0f, 0.0f } },


  { { -side2, -side2, -side2 },{ 0.0f, 0.0f, -1.0f } },
  { { -side2, side2, -side2 },{ 0.0f, 0.0f, -1.0f } },
  { { side2, side2, -side2 },{ 0.0f, 0.0f, -1.0f } },
  { { side2, -side2, -side2 },{ 0.0f, 0.0f, -1.0f } },


  { { -side2, -side2, side2 },{ -1.0f, 0.0f, 0.0f } },
  { { -side2, side2, side2 },{ -1.0f, 0.0f, 0.0f } },
  { { -side2, side2, -side2 },{ -1.0f, 0.0f, 0.0f } },
  { { -side2, -side2, -side2 },{ -1.0f, 0.0f, 0.0f } },


  { { -side2, -side2, side2 },{ 0.0f, -1.0f, 0.0f } },
  { { -side2, -side2, -side2 },{ 0.0f, -1.0f, 0.0f } },
  { { side2, -side2, -side2 },{ 0.0f, -1.0f, 0.0f } },
  { { side2, -side2, side2 },{ 0.0f, -1.0f, 0.0f } },


  { { -side2, side2, side2 },{ 0.0f, 1.0f, 0.0f } },
  { { side2, side2, side2 },{ 0.0f, 1.0f, 0.0f } },
  { { side2, side2, -side2 },{ 0.0f, 1.0f, 0.0f } },
  { { -side2, side2, -side2 },{ 0.0f, 1.0f, 0.0f } },
};
const std::vector<uint16_t> indices =
{
  0, 1, 2, 0, 2, 3,
  4, 5, 6, 4, 6, 7,
  8, 9, 10, 8, 10, 11,
  12, 13, 14, 12, 14, 15,
  16, 17, 18, 16, 18, 19,
  20, 21, 22, 20, 22, 23
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
      uint32_t mvpBufferSize = sizeof(UniformBufferObject);
      _uniformBufferMVP = _device->createBuffer( mvpBufferSize, 
        vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible | 
          vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    DescriptorSetLayoutBinding mvpDescriptor( 0, vk::DescriptorType::eUniformBuffer, 
      vk::ShaderStageFlagBits::eVertex );
    dslbs.push_back( mvpDescriptor );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    std::array<vk::DescriptorPoolSize, 1> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;
    DescriptorBufferInfo buffInfo( _uniformBufferMVP, 0, sizeof( glm::mat4 ) );
    WriteDescriptorSet w( _descriptorSet, 0, 0, 
      vk::DescriptorType::eUniformBuffer, 1, nullptr, buffInfo );
    wdss.push_back( w );
    _device->updateDescriptorSets( wdss, {} );

    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule = _device->createShaderModule( LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/cube_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    std::shared_ptr<ShaderModule> fragmentShaderModule = _device->createShaderModule( LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/cube_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
    PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );
    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos) ),
      vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color) ) }
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


    _pipeline = _device->createGraphicsPipeline( pipelineCache, {}, { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, _renderPass );
  }
  void updateUniformBuffers( )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    vk::Device device = static_cast<vk::Device>(*_device);

    uint32_t mvpBufferSize = sizeof(UniformBufferObject);
    void* data = _uniformBufferMVP->map( 0, mvpBufferSize );
    memcpy( data, &ubo, sizeof(ubo) );
    _uniformBufferMVP->unmap( );

    //std::cout<<glm::to_string(mvpc)<<std::endl;
  }

  void doPaint( ) override
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
    commandBuffer->setViewport( 0, vk::Viewport( 0.0f, 0.0f, ( float ) _defaultFramebuffer->getExtent( ).width, ( float ) _defaultFramebuffer->getExtent( ).height, 0.0f, 1.0f ) );
    commandBuffer->setScissor( 0, vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ) );
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
        glfwSetWindowShouldClose( getWindow( )->getWindow( ), GLFW_TRUE );
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
    VulkanApp* app = new MyApp( "Cube Indexed", 800, 600 );

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
  system( "PAUSE" );
  return 0;
}