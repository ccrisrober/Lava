#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

struct Vertex
{
  glm::vec3 position;
};

std::vector<Vertex> vertices;

#define MAXPOINTS 1000   // Change this to increment num of points
#define RANDOM_POINT 0.02

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  float time;
  float up;
  float beta;
} ubo;

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Buffer> _vertexBuffer;
  std::shared_ptr<Pipeline> _pipeline;
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<DescriptorSet> _descriptorSet;
  std::shared_ptr<Texture2D> tex;

  struct
  {
    std::shared_ptr<Buffer> vertexShader;
  } uniformBuffers;

  void createPoints( void )
  {
    srand( time( nullptr ) );
    for ( int i = 0; i < MAXPOINTS; ++i )
    {
      glm::vec3 point;
      point.x = ( ( float ) ( std::rand( ) % 1000 ) ) * RANDOM_POINT;
      point.y = ( ( float ) ( std::rand( ) % 1000 ) ) * RANDOM_POINT;
      point.z = ( ( float ) ( std::rand( ) % 1000 ) ) * RANDOM_POINT;
      vertices.push_back( Vertex{ point } );
    }
  }

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    createPoints( );

    uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
    _vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
    _vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

    // UNIFORM BUFFER
    uniformBuffers.vertexShader = _device->createBuffer( sizeof( ubo ),
      vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive, 
      nullptr, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );

    // updateUniforms( );
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "butterfly.png" ), commandPool, _graphicsQueue );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    dslbs.push_back( DescriptorSetLayoutBinding( 0, 
      vk::DescriptorType::eUniformBuffer, 
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry ) );
    dslbs.push_back( DescriptorSetLayoutBinding( 1, 
      vk::DescriptorType::eCombinedImageSampler, 
      vk::ShaderStageFlagBits::eFragment ) );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( 
      vk::DescriptorType::eUniformBuffer, 1 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( 
      vk::DescriptorType::eCombinedImageSampler, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( 
      descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;
    wdss.push_back( WriteDescriptorSet( _descriptorSet, 0, 0, 
      vk::DescriptorType::eUniformBuffer, 1, nullptr, 
      DescriptorBufferInfo( uniformBuffers.vertexShader, 0, 
        sizeof( ubo )
      )
    ) );
    wdss.push_back( WriteDescriptorSet( 
      _descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 
      1, DescriptorImageInfo( 
        vk::ImageLayout::eGeneral, 
        std::make_shared<vk::ImageView>( tex->view ), 
        std::make_shared<vk::Sampler>( tex->sampler )
      ), nullptr
    ) );
    _device->updateDescriptorSets( wdss, {} );

    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule = 
      _device->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE + 
          std::string("butterflies_vert.spv"), vk::ShaderStageFlagBits::eVertex );
    std::shared_ptr<ShaderModule> geomShaderModule = 
      _device->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE + 
          std::string( "butterflies_geom.spv" ), vk::ShaderStageFlagBits::eGeometry );
    std::shared_ptr<ShaderModule> fragmentShaderModule = 
      _device->createShaderModule( LAVA_EXAMPLES_SPV_ROUTE + 
          std::string( "butterflies_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main" );
    PipelineShaderStageCreateInfo geomStage( vk::ShaderStageFlagBits::eGeometry, geomShaderModule, "main" );
    PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main" );
    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, { 
      vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, position ) )
    } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::ePointList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } ); // Dynamic viewport and scissors
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true, false, vk::PolygonMode::eFill, 
      vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, 
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, 
      vk::DynamicState::eScissor } );


    _pipeline = _device->createGraphicsPipeline( pipelineCache, {}, 
      { vertexStage, geomStage, fragmentStage }, 
      vertexInput, assembly, nullptr, viewport, rasterization, multisample, 
      depthStencil, colorBlend, dynamic, _pipelineLayout, _renderPass );
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    ubo.model = glm::mat4( 1.0f );
    //ubo.model = glm::translate( ubo.model, glm::vec3( 0.0f, 0.0f, 25.0f ) );
    //ubo.model = glm::rotate(ubo.model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    ubo.time = time;
    ubo.up = 0.01f * std::sin( ubo.time );
    ubo.beta = glm::radians(-60.0f);

    std::cout << ubo.time << std::endl;

    vk::Device device = static_cast<vk::Device>(*_device);

    uint32_t mvpBufferSize = sizeof(UniformBufferObject);
    void* data = uniformBuffers.vertexShader->map( 0, mvpBufferSize );
    memcpy( data, &ubo, sizeof(ubo) );
    uniformBuffers.vertexShader->unmap( );

    //std::cout<<glm::to_string(mvpc)<<std::endl;
  }
  void doPaint( void ) override
  {
    std::cout << "Rendering" << std::endl;
    updateUniformBuffers( );

    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ),
    { vk::ClearValue( ccv ), vk::ClearValue( vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( _pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      _pipelineLayout, 0, { _descriptorSet }, nullptr );
    commandBuffer->bindVertexBuffer( 0, _vertexBuffer, 0 );
    commandBuffer->setViewport( 0, vk::Viewport( 0.0f, 0.0f, ( float ) _defaultFramebuffer->getExtent( ).width, ( float ) _defaultFramebuffer->getExtent( ).height, 0.0f, 1.0f ) );
    commandBuffer->setScissor( 0, vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ) );
    commandBuffer->draw( uint32_t( vertices.size( ) ), 1, 0, 0 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent(int key, int scancode, int action, int mods)
  {
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
      switch (action)
      {
      case GLFW_PRESS:
        glfwSetWindowShouldClose(getWindow()->getWindow( ), GLFW_TRUE);
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

void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main( void )
{
  try
  {
    //if (glfwInit())
    //{
    VulkanApp* app = new MyApp( "Butterflies", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      // app->waitEvents( );
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