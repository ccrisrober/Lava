#include <lava/lava.h>
using namespace lava;

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<CommandPool> commandPool;
  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    // create a command pool for command buffer allocation
    commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
      _device->createDescriptorSetLayout( dslbs );
    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    // init pipeline
    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquadUV_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    PipelineVertexInputStateCreateInfo vertexInput( {}, {} );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, 
      vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, false, false, 
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, 
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( 
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, 
      stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, 
      vk::DynamicState::eScissor } );


    pipeline = _device->createGraphicsPipeline( pipelineCache, {}, 
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _renderPass );
  }
  void doPaint( void ) override
  {
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 }, 
        _defaultFramebuffer->getExtent( ) ),
    { vk::ClearValue( ccv ), vk::ClearValue(
      vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( pipeline );
    
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    
    commandBuffer->draw( 4, 1, 0, 0 );
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

void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "FullQuadNoTexture", 800, 600 );

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