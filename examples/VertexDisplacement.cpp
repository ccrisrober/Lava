#include <lava/lava.h>
using namespace lava;

#include <routes.h>

struct
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
  float time;
} uboVS;

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 normal;
};

class MyApp : public VulkanApp
{
public:
  struct Pipelines
  {
    std::shared_ptr<Pipeline> solid;
  } pipelines;

  std::shared_ptr<Buffer> _uniformBufferMVP;
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<DescriptorSet> _descriptorSet;

  std::shared_ptr<VertexBuffer> _vertexBuffer;
  std::shared_ptr<IndexBuffer> _indexBuffer;
  std::shared_ptr<Texture2D> tex;

  MyApp( char const* title, uint32_t width, uint32_t height, const char* meshFile )
    : VulkanApp( title, width, height )
  {
    float t = ( 1.0f + std::sqrt( 5.0f ) ) / 2.0f;

    std::vector<float> verts = {
      -1.0f,  t,  0.0f,
      1.0f,  t,  0.0f,
      -1.0f, -t,  0.0f,
      1.0f, -t,  0.0f,

      0.0f, -1.0f,  t,
      0.0f,  1.0f,  t,
      0.0f, -1.0f, -t,
      0.0f,  1.0f, -t,

      t,  0.0f, -1.0f,
      t,  0.0f,  1.0f,
      -t,  0.0f, -1.0f,
      -t,  0.0f,  1.0f
    };
    std::vector<uint32_t> indices = {
      0, 11,   5,
      0,  5,   1,
      0,  1,   7,
      0,  7,  10,
      0, 10,  11,
      1,  5,   9,
      5, 11,   4,
      11, 10,   2,
      10,  7,   6,
      7,  1,   8,
      3,  9,   4,
      3,  4,   2,
      3,  2,   6,
      3,  6,   8,
      3,  8,   9,
      4,  9,   5,
      2,  4,  11,
      6,  2,  10,
      8,  6,   7,
      9,  8,   1
    };
    std::vector<float> norms;

    float radius = 5.0f;
    uint32_t subdivisions = 1;

    // Normalize
    for ( unsigned int i = 0, size = verts.size( ); i < size; i += 3 )
    {
      float mod = std::sqrt( verts[ i ] * verts[ i ] + verts[ i + 1 ] * verts[ i + 1 ] +
        verts[ i + 2 ] * verts[ i + 2 ] );
      float nX = verts[ i ] / mod;
      float nY = verts[ i + 1 ] / mod;
      float nZ = verts[ i + 2 ] / mod;
      norms.push_back( nX );
      norms.push_back( nY );
      norms.push_back( nZ );

      verts[ i ] *= radius / mod;
      verts[ i + 1 ] *= radius / mod;
      verts[ i + 2 ] *= radius / mod;
    }

    std::map<std::string, unsigned int> _pointsCache;

    auto midPoint( [&]( unsigned int a, unsigned int b )
    {
      std::string key = indices[ a ] < indices[ b ] ?
        std::to_string( indices[ a ] ) + ":" + std::to_string( indices[ b ] ) :
        std::to_string( indices[ b ] ) + ":" + std::to_string( indices[ a ] );

      std::map<std::string, unsigned int>::iterator r = _pointsCache.find( key );
      if ( r != _pointsCache.end( ) )
      {
        return r->second;
      }

      unsigned int index = verts.size( ) / 3;
      verts.push_back( ( verts[ indices[ a ] * 3 ] + verts[ indices[ b ] * 3 ] ) * 0.5f );
      verts.push_back( ( verts[ indices[ a ] * 3 + 1 ] + verts[ indices[ b ] * 3 + 1 ] ) * 0.5f );
      verts.push_back( ( verts[ indices[ a ] * 3 + 2 ] + verts[ indices[ b ] * 3 + 2 ] ) * 0.5f );

      float mod = std::sqrt( verts[ index * 3 ] *
        verts[ index * 3 ] + verts[ index * 3 + 1 ] *
        verts[ index * 3 + 1 ] + verts[ index * 3 + 2 ] * verts[ index * 3 + 2 ] );
      float nX = verts[ index * 3 ] / mod;
      float nY = verts[ index * 3 + 1 ] / mod;
      float nZ = verts[ index * 3 + 2 ] / mod;

      norms.push_back( nX );
      norms.push_back( nY );
      norms.push_back( nZ );

      verts[ index * 3 ] *= radius / mod;
      verts[ index * 3 + 1 ] *= radius / mod;
      verts[ index * 3 + 2 ] *= radius / mod;

      _pointsCache[ key ] = index;
      return index;
    } );

    // Regenerate indices
    for ( unsigned int ir = 0; ir < subdivisions; ++ir )
    {
      std::vector<unsigned int> new_el;
      for ( unsigned int i = 0, size = indices.size( ); i < size; i += 3 )
      {
        unsigned int midA = midPoint( i, i + 1 );
        unsigned int midB = midPoint( i + 1, i + 2 );
        unsigned int midC = midPoint( i + 2, i );
        new_el.push_back( indices[ i ] );
        new_el.push_back( midA );
        new_el.push_back( midC );
        new_el.push_back( indices[ i + 1 ] );
        new_el.push_back( midB );
        new_el.push_back( midA );
        new_el.push_back( indices[ i + 2 ] );
        new_el.push_back( midC );
        new_el.push_back( midB );
        new_el.push_back( midA );
        new_el.push_back( midB );
        new_el.push_back( midC );
      }
      indices = new_el;
    }

    std::vector<Vertex> vertices;
    for ( int i = 0; i < verts.size( ); i+= 3 )
    {
      vertices.push_back( Vertex{ glm::vec3(
        verts[ i ],
        verts[ i + 1 ],
        verts[ i + 2 ]
      ), glm::vec3(
        norms[ i ],
        norms[ i + 1 ],
        norms[ i + 2 ]
      ) } );
    }


    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      _vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      _vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( sizeof( indices[ 0 ] ) );
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
    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "green.png" ), commandPool, _graphicsQueue );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    dslbs.push_back( DescriptorSetLayoutBinding( 0,
      vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex ) );
    dslbs.push_back( DescriptorSetLayoutBinding( 1,
      vk::DescriptorType::eCombinedImageSampler,
      vk::ShaderStageFlagBits::eFragment ) );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule = 
      _device->createShaderModule( 
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "vertexDisplacement_vert.spv" ), 
        vk::ShaderStageFlagBits::eVertex
      );
    std::shared_ptr<ShaderModule> fragmentShaderModule = 
      _device->createShaderModule( 
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "vertexDisplacement_frag.spv" ), 
        vk::ShaderStageFlagBits::eFragment
      );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
    PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, 
            vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, position )
          ),
          vk::VertexInputAttributeDescription( 1, 0, 
            vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, normal )
          )
        }
    );
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


    pipelines.solid = _device->createGraphicsPipeline( pipelineCache, {}, { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, _renderPass );

    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize(
      vk::DescriptorType::eUniformBuffer, 1 );
    poolSize[ 1 ] = vk::DescriptorPoolSize(
      vk::DescriptorType::eCombinedImageSampler, 1 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss;
    DescriptorBufferInfo buffInfo( _uniformBufferMVP, 0, sizeof( uboVS ) );
    WriteDescriptorSet w( _descriptorSet, 0, 0, 
      vk::DescriptorType::eUniformBuffer, 1, nullptr, buffInfo );
    wdss.push_back( w );
    wdss.push_back( WriteDescriptorSet(
      _descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler,
      1, DescriptorImageInfo(
        vk::ImageLayout::eGeneral,
        std::make_shared<vk::ImageView>( tex->view ),
        std::make_shared<vk::Sampler>( tex->sampler )
      ), nullptr
    ) );
    _device->updateDescriptorSets( wdss, {} );

  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    uboVS.model = glm::mat4( 1.0f );
    //uboVS.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 0.5f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

    uboVS.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    uboVS.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
    uboVS.proj[1][1] *= -1;

    uboVS.time = time * .25f;

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

    commandBuffer->bindGraphicsPipeline( pipelines.solid );

    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      _pipelineLayout, 0, { _descriptorSet }, nullptr );
    commandBuffer->setViewport( 0, vk::Viewport( 0.0f, 0.0f, ( float ) _defaultFramebuffer->getExtent( ).width, ( float ) _defaultFramebuffer->getExtent( ).height, 0.0f, 1.0f ) );
    commandBuffer->setScissor( 0, vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ) );

    _vertexBuffer->bind( commandBuffer );
    _indexBuffer->bind( commandBuffer );

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

int main( int argc, char** argv )
{
  try
  {
    VulkanApp* app = new MyApp( "Mesh loading", 800, 600, 
      (LAVA_EXAMPLES_MESHES_ROUTE + std::string( "monkey.obj_" )).c_str( ) );

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
  system( "PAUSE" );
  return 0;
}