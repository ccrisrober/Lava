#include <lava/lava.h>
using namespace lava;

#include <routes.h>

namespace material
{
  class UVMaterial: public lava::engine::Material
  {
  public:
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

    virtual void configure( const std::string& dir,
      std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass )
    {
      // MVP buffer
      {
        uint32_t mvpBufferSize = sizeof( uboVS );
        uniformBufferMVP = dev->createBuffer( mvpBufferSize,
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive, nullptr,
          vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent );
      }

      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs = 
      {
        DescriptorSetLayoutBinding( 0, 
          vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex
        ),
        DescriptorSetLayoutBinding( 1, 
          vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        )
      };
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
        dev->createDescriptorSetLayout( dslbs );

      _pipelineLayout = dev->createPipelineLayout( descriptorSetLayout, nullptr );

      std::array<vk::DescriptorPoolSize, 2> poolSize =
      {
        // Binding 0: MVP uniform buffer
        vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
        // Binding 1: Texture
        vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
      };
      std::shared_ptr<DescriptorPool> descriptorPool = 
        dev->createDescriptorPool( {}, 1, poolSize );


      // Init descriptor set
      descriptorSet = dev->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet( 
          descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr,
          DescriptorBufferInfo( 
            uniformBufferMVP, 0, sizeof( uboVS )
          )
        ),
        WriteDescriptorSet( 
          descriptorSet, 1, 0, 
          vk::DescriptorType::eCombinedImageSampler, 1,
          tex->descriptor, nullptr
        )
      };
      dev->updateDescriptorSets( wdss, {} );

      // init pipeline
      std::shared_ptr<PipelineCache> pipelineCache = dev->createPipelineCache( 0, nullptr );
      PipelineShaderStageCreateInfo vertexStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      PipelineShaderStageCreateInfo fragmentStage = dev->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );
      vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
        vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos ) ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord ) ) }
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


      _pipeline = dev->createGraphicsPipeline( pipelineCache, {}, 
      { vertexStage, fragmentStage }, vertexInput, assembly, 
        nullptr, viewport, rasterization, multisample, 
        depthStencil, colorBlend, dynamic,
        _pipelineLayout, renderPass );
    }
    virtual void bind( std::shared_ptr< CommandBuffer > cmd )
    {
      lava::engine::Material::bind( cmd );
      if ( descriptorSet )
      {
        cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
          _pipelineLayout, 0, { descriptorSet }, nullptr );
      }
    }
    std::shared_ptr<Texture2D> tex;
    std::shared_ptr<Buffer> uniformBufferMVP;
  protected:
    std::shared_ptr<DescriptorSet> descriptorSet;
  };
}

const float side = 1.0f;
const float side2 = side / 2.0f;
const std::vector<material::UVMaterial::Vertex> vertices =
{
  {{-side2, -side2,  side2}, {0.0f, 0.0f}},
  {{ side2, -side2,  side2}, {1.0f, 0.0f}},
  {{-side2,  side2,  side2}, {0.0f, 1.0f}},
  {{ side2,  side2,  side2}, {1.0f, 1.0f}},

  {{-side2, -side2, -side2}, {0.0f, 0.0f}},
  {{ side2, -side2, -side2}, {1.0f, 0.0f}},
  {{-side2,  side2, -side2}, {0.0f, 1.0f}},
  {{ side2,  side2, -side2}, {1.0f, 1.0f}},

  {{ side2, -side2, -side2}, {0.0f, 0.0f}},
  {{ side2, -side2,  side2}, {1.0f, 0.0f}},
  {{ side2,  side2, -side2}, {0.0f, 1.0f}},
  {{ side2,  side2,  side2}, {1.0f, 1.0f}},

  {{-side2, -side2, -side2}, {0.0f, 0.0f}},
  {{-side2, -side2,  side2}, {1.0f, 0.0f}},
  {{-side2,  side2, -side2}, {0.0f, 1.0f}},
  {{-side2,  side2,  side2}, {1.0f, 1.0f}},

  {{-side2,  side2, -side2}, {0.0f, 0.0f}},
  {{-side2,  side2,  side2}, {1.0f, 0.0f}},
  {{ side2,  side2, -side2}, {0.0f, 1.0f}},
  {{ side2,  side2,  side2}, {1.0f, 1.0f}},

  {{-side2, -side2, -side2}, {0.0f, 0.0f}},
  {{-side2, -side2,  side2}, {1.0f, 0.0f}},
  {{ side2, -side2, -side2}, {0.0f, 1.0f}},
  {{ side2, -side2,  side2}, {1.0f, 1.0f}} 
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
  std::shared_ptr<VertexBuffer> vertexBuffer;
  std::shared_ptr<IndexBuffer> indexBuffer;
  std::shared_ptr<material::UVMaterial> material;

  std::shared_ptr<CommandPool> commandPool;
  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( material::UVMaterial::Vertex );
      vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( indices[ 0 ] );
      indexBuffer = std::make_shared<IndexBuffer>( _device, 
        vk::IndexType::eUint16, indices.size( ) );
      indexBuffer->writeData( 0, indexBufferSize, indices.data( ) );
    }

    material = std::make_shared<material::UVMaterial>( );

    // create a command pool for command buffer allocation
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    material->tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE + 
      std::string( "rubymatcap.jpg" ), commandPool, _graphicsQueue, 
      vk::Format::eR8G8B8A8Unorm );

    material->configure( LAVA_EXAMPLES_SPV_ROUTE, _device, _renderPass );
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    material->uboVS.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    material->uboVS.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    material->uboVS.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
    material->uboVS.proj[1][1] *= -1;

    material->uniformBufferMVP->writeData( 0, sizeof( material->uboVS ), &material->uboVS );
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

    material->bind( commandBuffer );

    vertexBuffer->bind( commandBuffer );
    indexBuffer->bind( commandBuffer );
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
    VulkanApp* app = new MyApp( "Cube UV", 800, 600 );

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