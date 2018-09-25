#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;

  pompeii::utility::ThreadPool threadPool;
  struct PerThreadData
  {
    std::shared_ptr< pompeii::CommandPool > commandPool;
    std::vector< std::shared_ptr< pompeii::CommandBuffer > > cmdBuffers;
  } perThreadData[ 8 ];
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

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
  const float side2 = side * 0.5f;

  const std::vector<Vertex> vertices =
  {
    { { -side2, -side2,  side2 },{ 0.0f, 0.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2,  side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { side2, -side2, -side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2, -side2 },{ 1.0f, 1.0f } },

    { { side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { -side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2,  side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2,  side2,  side2 },{ 1.0f, 0.0f } },
    { { side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { side2, -side2, -side2 },{ 0.0f, 1.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 1.0f } }
  };
  const std::vector<uint16_t> indices =
  {
    0,  1,  2,     1,  3,  2,
    4,  6,  5,     5,  6,  7,
    8, 10,  9,     9, 10, 11,
    12, 13, 14,    13, 15, 14,
    16, 17, 18,    17, 19, 18,
    20, 22, 21,    21, 22, 23,
  };

  void initResources( void ) override
  {
    auto device = _window->device( );

    {
      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );

      // Vertex buffer
        uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
        vertexBuffer = device->createBuffer( vertexBufferSize,
          vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal );
        vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ),
          vertices.data( ) } );

      // Index buffer
        uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );
        indexBuffer = device->createBuffer( indexBufferSize,
          vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal );
        indexBuffer->update<uint16_t>( cmd, 0, { uint32_t( indices.size( ) ),
          indices.data( ) } );

      cmd->end( );
      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // MVP buffer
    {
      mvpBuffer = device->createUniformBuffer( sizeof( uboVS ) );
    }

    tex = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "uv_checker.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "cubeUVPush_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    vk::PushConstantRange pushConstantRange(
      vk::ShaderStageFlagBits::eVertex, 0, sizeof( glm::vec3 )
    );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, pushConstantRange );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription(
        0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos )
      ),
      vk::VertexInputAttributeDescription(
        1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord )
      )
    } );

    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );
    ;
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      vk::PipelineColorBlendAttachmentState(
        false, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
      ), { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( )
    );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
    };
    auto dspPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( dspPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( mvpBuffer, 0, sizeof( uboVS ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );

    numworkers = std::thread::hardware_concurrency( );
    std::cout << "Numworkers: " << numworkers << std::endl;
    threadPool.setThreadCount( numworkers );

    for ( uint32_t i = 0; i < numworkers; ++i )
    {
      perThreadData[ i ].commandPool = device->createCommandPool( );
    }
  }

  uint32_t numworkers;

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::mat4( 1.0f );
    //uboVS.model = glm::scale( uboVS.model, glm::vec3( 0.1f ) );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 45.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

    uboVS.view = glm::lookAt( glm::vec3( 1.0f, 0.0f, MAX * 1.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );

    // Vulkan clip space has inverted Y and half Z.
    glm::mat4 clip = glm::mat4(
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.0f, 0.0f, 0.5f, 1.0f
    );
    uboVS.proj = clip * uboVS.proj;
    //uboVS.proj[ 1 ][ 1 ] *= -1;

    mvpBuffer->writeData( 0, sizeof( uboVS ), &uboVS );
  }
  const int MAX = 64;

  void nextFrame( void ) override
  {
    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const auto size = _window->swapchainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = size;

    auto renderPass = _window->renderPass( );
    auto fbo = _window->framebuffer( );

    // The primary command buffer does not contain any rendering commands
    // These are stored (and retrieved) from the secondary command buffers
    cmd->beginRenderPass(
      renderPass,
      fbo,
      rect, clearValues, vk::SubpassContents::eSecondaryCommandBuffers
    );

    /**
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );
    cmd->setViewportScissors( size );
    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, { } );

    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );

    for ( uint32_t i = 0; i < numworkers; ++i )
    {
      for ( int j = -MAX; j < MAX; j += 8 )
      {
        for ( int t = -MAX; t < MAX; t += 8 )
        {
          glm::vec3 newPosOffset( 0.0f );
          newPosOffset.x = t * 1.0f;
          newPosOffset.y = j * 1.0f;
          newPosOffset.z = i * 1.0f;
          cmd->pushConstants<glm::vec3>( *pipelineLayout,
            vk::ShaderStageFlagBits::eVertex, 0, newPosOffset );

          cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );
        }
      }
    }
    /**/


    /**/
    std::vector<std::shared_ptr<pompeii::CommandBuffer>> secondaryBuffers;
    for ( uint32_t i = 0; i < numworkers; ++i )
    {
      perThreadData[ i ].cmdBuffers.clear( );
    }
    for ( uint32_t i = 0; i < numworkers; ++i )
    {
      for ( int j = -MAX; j < MAX; j += 8 )
      {
        threadPool.workers[ i ]->addJob( [ = ] {
          std::shared_ptr<CommandBuffer> secondaryBuffer = perThreadData[ i ].
            commandPool->allocateCommandBuffer( vk::CommandBufferLevel::eSecondary );

          perThreadData[ i ].cmdBuffers.push_back( secondaryBuffer );

          secondaryBuffer->begin(
            vk::CommandBufferUsageFlagBits::eRenderPassContinue, 
            renderPass, 0, fbo );

          secondaryBuffer->bindGraphicsPipeline( pipeline );
          secondaryBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
            pipelineLayout, 0, { descriptorSet }, { } );

          secondaryBuffer->bindVertexBuffer( 0, vertexBuffer, 0 );
          secondaryBuffer->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );
          secondaryBuffer->setViewportScissors( size );

          for ( int t = -MAX; t < MAX; t += 8 )
          {
            glm::vec3 newPosOffset( 0.0f );
            newPosOffset.x = t * 1.0f;
            newPosOffset.y = j * 1.0f;
            newPosOffset.z = i * 1.0f;
            secondaryBuffer->pushConstants<glm::vec3>( *pipelineLayout,
              vk::ShaderStageFlagBits::eVertex, 0, newPosOffset );

            secondaryBuffer->drawIndexed( indices.size( ), 1, 0, 0, 1 );
          }

          secondaryBuffer->end( );

        } );
      }
    }
    threadPool.wait( );

    // Only submit if object is within the current view frustum
    for ( uint32_t i = 0; i < numworkers; ++i )
    {
      for ( uint32_t j = 0; j < perThreadData[ i ].cmdBuffers.size( ); ++j )
      {
        secondaryBuffers.push_back( perThreadData[ i ].cmdBuffers[ j ] );
      }
    }


    cmd->executeCommands( secondaryBuffers );
    /**/

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  std::shared_ptr< Texture2D > tex;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< Buffer > vertexBuffer;
  std::shared_ptr< Buffer > indexBuffer;
  std::shared_ptr< UniformBuffer > mvpBuffer;
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {

  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "ThreadPool", true );
  app.show( );
  return EXIT_SUCCESS;
}