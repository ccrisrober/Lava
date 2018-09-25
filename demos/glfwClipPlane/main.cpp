#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
using namespace pompeii;

#include <routes.h>

#include "../utils/Camera.h"

#define PI 3.14159

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
    camera = Camera( glm::vec3( 0.0f, 0.0f, 25.5f ) );
  }

  std::shared_ptr<pompeii::QueryPool> queryPool;
  std::shared_ptr<pompeii::Buffer> queryResultBuffer;

  // camera
  Camera camera;
  // timing
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 clipPlane;
  } ubo;

  struct Vertex
  {
    glm::vec3 pos;
  };

  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;

  void CreateTorus( float major, float minor,
    uint32_t slices, uint32_t stacks )
  {
    //uint32_t vertexCount = slices * stacks * 3;
    //vertices.reserve( vertexCount );

    //glm::vec3* position = &vertices;
    for ( uint32_t slice = 0; slice < slices; ++slice )
    {
      float theta = slice * 2.0f * PI / slices;
      for ( uint32_t stack = 0; stack < stacks; ++stack )
      {
        float phi = stack * 2.0f * PI / stacks;
        float beta = major + minor * cos( phi );
        vertices.push_back( Vertex{
          glm::vec3(
            cos( theta ) * beta,
            sin( theta ) * beta,
            sin( phi ) * minor
          )
        } );
      }
    }

    //uint32_t indexCount = slices * stacks * 6;
    //indices.reserve( indexCount );

    //uint32_t* index = &indices[0];
    uint32_t v = 0;
    for ( uint32_t i = 0; i < slices - 1; ++i )
    {
      for ( uint32_t j = 0; j < stacks; ++j )
      {
        uint32_t next = ( j + 1 ) % stacks;

        indices.push_back( v + next + stacks );
        indices.push_back( v + next );
        indices.push_back( v + j );

        indices.push_back( v + j );
        indices.push_back( v + j + stacks );
        indices.push_back( v + next + stacks );
      }
      v += stacks;
    }
    for ( uint32_t j = 0; j < stacks; ++j )
    {
      uint32_t next = ( j + 1 ) % stacks;
      indices.push_back( next );
      indices.push_back( v + next );
      indices.push_back( v + j );

      indices.push_back( v + j );
      indices.push_back( j );
      indices.push_back( next );
    }
  }

  // Passed query samples
  std::vector<uint64_t> passedSamples;

  void setupQueryPool( void )
  {
    auto device = _window->device( );

    queryPool = device->createOcclusionQuery( 1 );

    passedSamples.reserve( 1 );
    passedSamples.resize( 1 );
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    setupQueryPool( );
    queryResultBuffer = device->createBuffer( 1 * sizeof( uint64_t ), 
      vk::BufferUsageFlagBits::eUniformBuffer | 
      vk::BufferUsageFlagBits::eTransferDst, 
      vk::MemoryPropertyFlagBits::eHostVisible | 
      vk::MemoryPropertyFlagBits::eHostCoherent );

    CreateTorus( 8.0f, 2.0f, 40, 10 );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, vertexBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      auto stagingBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, indexBufferSize, indices.data( ) );

      indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );
      stagingBuffer->copy( cmd, indexBuffer, 0, 0, indexBufferSize );
      cmd->end( );

      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // MVP buffer
    uboUniform = device->createUniformBuffer( sizeof( ubo ) );
    ubo.clipPlane = glm::vec4( 0.0f, 1.0f, 0.0f, 0.0f );

    std::array<vk::DescriptorPoolSize, 1> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
    };

    auto descriptorPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex |
      vk::ShaderStageFlagBits::eGeometry
      )
    };
    auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "clipPlane_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto geomStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "clipPlane_geom.spv" ),
      vk::ShaderStageFlagBits::eGeometry
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "clipPlane_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    vk::VertexInputBindingDescription binding(
      0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
      offsetof( Vertex, pos )
      ) }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState,
      0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipelines.solid = device->createGraphicsPipeline(
      _window->pipelineCache( ), { }, { vertexStage, geomStage, fragmentStage },
      vertexInput, assembly, nullptr, viewport, rasterization, multisample,
      depthStencil, colorBlend, dynamic, pipelineLayout,
      _window->renderPass( ) );

    // Wireframe rendering pipeline
    if ( _window->physicalDevice( )->getDeviceFeatures( ).fillModeNonSolid )
    {
      rasterization.polygonMode = vk::PolygonMode::eLine;
      rasterization.lineWidth = 1.0f;

      rasterization.cullMode = vk::CullModeFlagBits::eNone;

      pipelines.wireframe = device->createGraphicsPipeline(
        _window->pipelineCache( ), { }, { vertexStage, geomStage, fragmentStage },
        vertexInput, assembly, nullptr, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, pipelineLayout,
        _window->renderPass( ) );
    }

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool,
      descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
      vk::DescriptorType::eUniformBuffer, 1, nullptr,
      DescriptorBufferInfo( uboUniform, 0, sizeof( ubo ) )
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    ubo.model = glm::mat4( 1.0f );
    ubo.model = glm::scale( ubo.model, glm::vec3( 0.05f ) );
    ubo.model = glm::rotate( glm::mat4( 1.0f ), time * 0.25f * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.view = camera.GetViewMatrix( );
    ubo.proj = glm::perspective( glm::radians( camera.Zoom ), 
      ( float ) size.width / ( float ) size.height, 0.1f, 100.0f );
    ubo.proj[ 1 ][ 1 ] *= -1;

    ubo.clipPlane.z = 7.0f * sin( time );

    uboUniform->set( &ubo );
  }

  void nextFrame( void ) override
  {
    /*if ( Input::isKeyPressed( pompeii::Keyboard::Key::Z ) )
    {
      enable_wire = false;
    }
    else if ( Input::isKeyPressed( pompeii::Keyboard::Key::X ) )
    {
      enable_wire = true;
    }*/

    const auto size = _window->swapchainImageSize( );

    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    auto cmd = _window->currentCommandBuffer( );

    // Reset query pool
    // Must be done outside of render pass
    cmd->resetQueryPool( queryPool, 0 /*startQuery*/, 2 /*queryCount*/ );

    vk::Rect2D rect;
    rect.extent = size;
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( enable_wire ? pipelines.wireframe :
      pipelines.solid );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );
    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );
    cmd->setViewportScissors( size );

    cmd->beginQuery( queryPool, 0, { } );
    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    cmd->endRenderPass( );
    cmd->endQuery( queryPool, 0 );
    
    cmd->copyQueryPoolResults( queryPool, 0, 1,
      queryResultBuffer, 0, 1 * sizeof( uint64_t ),
      vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait );

    _window->frameReady( );

    // Read query results for displaying in next frame
    getQueryResults( );
  }
  void getQueryResults( void )
  {
    passedSamples = queryPool->getResults<uint64_t>( 
      0, 1, passedSamples.size( ), sizeof( uint64_t ),
      // Store results a 64 bit values and wait until the results have been finished
      // If you don't want to wait, you can use VK_QUERY_RESULT_WITH_AVAILABILITY_BIT
      // which also returns the state of the result (ready) in the result
      vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait );
    std::cout << "vkGetQueryPoolResults data" << "\n";
    std::cout << "samples_passed = " << passedSamples[ 0 ] << "\n";

    // Read back query result from buffer
    uint64_t samples_passed_ptr[ 1 ];
    queryResultBuffer->read( &samples_passed_ptr );
    std::cout << "vkCmdCopyQueryPoolResults data" << "\n";
    std::cout << "samples_passed = " << samples_passed_ptr[ 0 ] << std::endl;

    assert( passedSamples[ 0 ] == samples_passed_ptr[ 0 ] );
  }
  bool enable_wire = false;

private:
  std::shared_ptr<Buffer> vertexBuffer;
  std::shared_ptr<Buffer> indexBuffer;

  struct Pipelines
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> wireframe;
  } pipelines;

  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<Buffer> uboUniform;
};

class VulkanWindow : public glfw::VulkanWindow
{
protected:
  MainWindowRenderer* _renderer;
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }
  virtual void keyEvent( int key, int act ) override
  {
    if ( act == GLFW_PRESS )
    {
      if ( key == GLFW_KEY_Z )
      {
        _renderer->enable_wire = false;
      }
      else if ( key == GLFW_KEY_X )
      {
        _renderer->enable_wire = true;
      }
    }
  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    _renderer = new MainWindowRenderer( this );
    return _renderer;
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "Clip planes", true );
  app.show( );
  return EXIT_SUCCESS;
}