#include <lava/lava.h>
using namespace lava;

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Pipeline> pipeline;

  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;

  std::shared_ptr<CommandPool> commandPool;
  std::shared_ptr<Buffer> texelBuffer;
  std::shared_ptr<lava::BufferView> texelView;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    auto devProp = _device->_physicalDevice->getDeviceProperties( );
    if ( ( devProp.limits.maxTexelBufferElements < 4 * 3 ) )
    {
      std::cout << "maxTexelBufferElements too small" << std::endl;
      exit( -1 );
    }

    vk::FormatProperties props = _device->_physicalDevice->getFormatProperties( vk::Format::eR32G32B32Sfloat );
    if ( !( props.bufferFeatures & vk::FormatFeatureFlagBits::eUniformTexelBuffer ) )
    {
      std::cout << "R32G32B32_SFLOAT format unsupported for texel buffer" << std::endl;
      exit( -1 );
    }
    glm::vec3 texels[] = { 
      glm::vec3( 1.0f, 0.0f, 0.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ),
      glm::vec3( 0.0f, 0.0f, 1.0f )
    };

    // Texel buffer
    {
      texelBuffer = _device->createBuffer( sizeof( texels ),
        vk::BufferUsageFlagBits::eUniformTexelBuffer,
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent
      );
    }
    texelBuffer->writeData( 0, sizeof( texels ), &texels );

    texelView =
      std::make_shared<lava::BufferView>( texelBuffer, 
        vk::Format::eR32G32B32Sfloat,
        0, sizeof( texels )
      );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformTexelBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout =
      _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    std::array<vk::DescriptorPoolSize, 1> poolSize =
    {
      // Binding 0: Texel buffer
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformTexelBuffer, 1 )
    };
    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 1, poolSize );

    // Init descriptor set
    descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformTexelBuffer,
        1, nullptr,
        DescriptorBufferInfo(
          texelBuffer, 0, sizeof( texels )
        ), texelView
      )
    };
    _device->updateDescriptorSets( wdss, {} );


    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage =
      _device->createShaderPipelineShaderStage( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "texelBuffer_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    PipelineShaderStageCreateInfo fragmentStage =
      _device->createShaderPipelineShaderStage( LAVA_EXAMPLES_SPV_ROUTE +
        std::string( "texelBuffer_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, 
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { { } }, { { } } );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );
    PipelineMultisampleStateCreateInfo multisample( 
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, 
      vk::CompareOp::eAlways, 0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, 
      true, true, vk::CompareOp::eLessOrEqual, false, false, 
      stencilOpState, stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, 
      vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( { 
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = _device->createGraphicsPipeline( pipelineCache, { },
    { vertexStage, fragmentStage },
      vertexInput, assembly, nullptr, viewport, rasterization, multisample,
      depthStencil, colorBlend, dynamic, pipelineLayout, _renderPass );
  }

  void doPaint( void ) override
  {
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

    commandBuffer->bindGraphicsPipeline( pipeline );

    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );
    
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    
    commandBuffer->draw( 3, 1, 0, 0 );
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
    VulkanApp* app = new MyApp( "Texel Buffer triangle", 800, 600 );

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