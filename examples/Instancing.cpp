#define _USE_MATH_DEFINES
#include <cmath>

#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include <random>

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
const std::vector<Vertex> vertices =
{
  { { -side2, -side2,  side2 }, { 0.0f, 0.0f } },
  { {  side2, -side2,  side2 }, { 1.0f, 0.0f } },
  { { -side2,  side2,  side2 }, { 0.0f, 1.0f } },
  { {  side2,  side2,  side2 }, { 1.0f, 1.0f } },

  { { -side2, -side2, -side2 }, { 0.0f, 0.0f } },
  { {  side2, -side2, -side2 }, { 1.0f, 0.0f } },
  { { -side2,  side2, -side2 }, { 0.0f, 1.0f } },
  { {  side2,  side2, -side2 }, { 1.0f, 1.0f } },

  { {  side2, -side2, -side2 }, { 0.0f, 0.0f } },
  { {  side2, -side2,  side2 }, { 1.0f, 0.0f } },
  { {  side2,  side2, -side2 }, { 0.0f, 1.0f } },
  { {  side2,  side2,  side2 }, { 1.0f, 1.0f } },

  { { -side2, -side2, -side2 }, { 0.0f, 0.0f } },
  { { -side2, -side2,  side2 }, { 1.0f, 0.0f } },
  { { -side2,  side2, -side2 }, { 0.0f, 1.0f } },
  { { -side2,  side2,  side2 }, { 1.0f, 1.0f } },

  { { -side2,  side2, -side2 }, { 0.0f, 0.0f } },
  { { -side2,  side2,  side2 }, { 1.0f, 0.0f } },
  { {  side2,  side2, -side2 }, { 0.0f, 1.0f } },
  { {  side2,  side2,  side2 }, { 1.0f, 1.0f } },

  { { -side2, -side2, -side2 }, { 0.0f, 0.0f } },
  { { -side2, -side2,  side2 }, { 1.0f, 0.0f } },
  { {  side2, -side2, -side2 }, { 0.0f, 1.0f } },
  { {  side2, -side2,  side2 }, { 1.0f, 1.0f } } 
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

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define INSTANCE_COUNT 8192

struct InstanceData
{
  glm::vec3 pos;
};

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<VertexBuffer> vertexBuffer;
  std::shared_ptr<IndexBuffer> indexBuffer;
  std::shared_ptr<Buffer> uniformBufferMVP;
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<Texture2D> tex;

  std::shared_ptr<VertexBuffer> _instanceBuffer;
    std::shared_ptr<CommandPool> commandPool;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    // create a command pool for command buffer allocation
    commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );
      indexBuffer = std::make_shared<IndexBuffer>( _device, 
        vk::IndexType::eUint16, indices.size( ) );
      indexBuffer->writeData( 0, indexBufferSize, indices.data( ) );
    }

    // MVP buffer
    {
      uint32_t mvpBufferSize = sizeof(uboVS);
      uniformBufferMVP = _device->createBuffer( mvpBufferSize, 
        vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible | 
          vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE + 
      std::string( "random.png" ), commandPool, _graphicsQueue, 
      vk::Format::eR8G8B8A8Unorm );

    // Instancing buffer
    {
      std::vector<InstanceData> instanceData;
      instanceData.resize( INSTANCE_COUNT );

      std::mt19937 rndGenerator( time( nullptr ) );
      std::uniform_real_distribution<float> uniformDist( 0.0, 1.0 );
      for ( uint32_t i = 0, l = INSTANCE_COUNT / 2; i < l; ++i )
      {
        glm::vec2 ring0{ 7.0f, 11.0f };
        glm::vec2 ring1{ 18.0f, 25.0f };

        float rho, theta;

        // Inner ring
        rho = sqrt( ( pow( ring0[ 1 ], 2.0f ) - pow( ring0[ 0 ], 2.0f ) ) * uniformDist( rndGenerator ) + pow( ring0[ 0 ], 2.0f ) );
        theta = 2.0 * M_PI * uniformDist( rndGenerator );
        instanceData[ i ].pos = glm::vec3( rho*cos( theta ), uniformDist( rndGenerator ) * 0.5f - 0.25f, rho*sin( theta ) );

        // Outer ring
        rho = sqrt( ( pow( ring1[ 1 ], 2.0f ) - pow( ring1[ 0 ], 2.0f ) ) * uniformDist( rndGenerator ) + pow( ring1[ 0 ], 2.0f ) );
        theta = 2.0 * M_PI * uniformDist( rndGenerator );
        instanceData[ i + INSTANCE_COUNT / 2 ].pos = glm::vec3( rho*cos( theta ), uniformDist( rndGenerator ) * 0.5f - 0.25f, rho*sin( theta ) );
      }

      uint32_t instancingBufferSize = instanceData.size( ) * sizeof( InstanceData );
      _instanceBuffer = std::make_shared<VertexBuffer>( _device, instancingBufferSize );
      _instanceBuffer->writeData( 0, instancingBufferSize, instanceData.data( ) );
    }



    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    DescriptorSetLayoutBinding mvpDescriptor( 0, vk::DescriptorType::eUniformBuffer, 
      vk::ShaderStageFlagBits::eVertex );
    dslbs.push_back( mvpDescriptor );
    DescriptorSetLayoutBinding mvpDescriptor2( 1, vk::DescriptorType::eCombinedImageSampler, 
      vk::ShaderStageFlagBits::eFragment );
    dslbs.push_back( mvpDescriptor2 );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

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
          uniformBufferMVP, 0, sizeof( glm::mat4 )
        )
      ),
      WriteDescriptorSet( descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };

    _device->updateDescriptorSets( wdss, {} );

    // init pipeline
    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "instancing_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "instancing_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    
    
    PipelineVertexInputStateCreateInfo vertexInput( {
      // Binding point 0: Mesh vertex layout description at per-vertex rate
      vk::VertexInputBindingDescription( VERTEX_BUFFER_BIND_ID, 
        sizeof( Vertex ), vk::VertexInputRate::eVertex ),
      // Binding point 1: Instanced data at per-instance rate
      vk::VertexInputBindingDescription( INSTANCE_BUFFER_BIND_ID, 
        sizeof( InstanceData ), vk::VertexInputRate::eInstance )
    }, {
      vk::VertexInputAttributeDescription( 0, VERTEX_BUFFER_BIND_ID, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos) ),
      vk::VertexInputAttributeDescription( 1, VERTEX_BUFFER_BIND_ID, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord) ),
      vk::VertexInputAttributeDescription( 2, INSTANCE_BUFFER_BIND_ID, vk::Format::eR32G32B32Sfloat, offsetof(InstanceData, pos) )
    });

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
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    uboVS.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    uboVS.view = glm::lookAt(glm::vec3(2.0f, 75.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    uboVS.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
    uboVS.proj[1][1] *= -1;

    uniformBufferMVP->writeData( 0, sizeof( uboVS), &uboVS );
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

    // Binding point 0 : Mesh vertex buffer
    commandBuffer->bindVertexBuffer( VERTEX_BUFFER_BIND_ID, vertexBuffer, 0 );
    // Binding point 1 : Instance data buffer
    commandBuffer->bindVertexBuffer( INSTANCE_BUFFER_BIND_ID, _instanceBuffer, 0 );

    indexBuffer->bind( commandBuffer );
    
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    
    commandBuffer->drawIndexed( indices.size( ), INSTANCE_COUNT, 0, 0, 0 );
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
    VulkanApp* app = new MyApp( "Instancing", 800, 600 );

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