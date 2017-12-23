#include <lava/lava.h>
using namespace lava;
#include <routes.h>

#define VK_USE_PLATFORM_WIN32_KHR

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
  }
  
  void initResources( void ) override
  {
    auto device = _window->device( );

    tex = std::make_shared<Texture2D>( device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "uv_checker.png" ), _window->graphicsCommandPool(), 
      _window->graphicQueue(), vk::Format::eR8G8B8A8Unorm );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding(
        0, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descSetLayout, nullptr );

    auto vertexStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput( {}, {} );

    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true, false,
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( { 
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache, { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->defaultRenderPass( )
    );

    std::shared_ptr<DescriptorPool> descriptorPool =
      device->createDescriptorPool( 1, {
        { vk::DescriptorType::eCombinedImageSampler, 1 }
      } );

    // Init descriptor set
    descSet = device->allocateDescriptorSet( descriptorPool, descSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descSet, 0, 0, 
        vk::DescriptorType::eCombinedImageSampler, 1, tex->descriptor, nullptr
      )
    };

    device->updateDescriptorSets( wdss, {} );
  }

  void releaseResources( void ) override
  {
    if ( descSetLayout )
    {
      descSetLayout.reset( );
    }
    if ( descSet )
    {
      descSet.reset( );
    }
    if ( pipelineLayout )
    {
      pipelineLayout.reset( );
    }
    if ( pipeline )
    {
      pipeline.reset( );
    }
    if ( tex )
    {
      tex.reset( );
    }
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }
    static int i = 0;

    _window->setWindowTitle( std::string( "Step: " ) + std::to_string( i ) );

    ++i;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    _green = sin( time ) / 2.0f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, _green, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil  = vk::ClearDepthStencilValue(  1.0f, 0 );

    secondaryCmd = _window->_cmdPool->allocateCommandBuffer( vk::CommandBufferLevel::eSecondary );

    vk::CommandBufferInheritanceInfo inheritInfo;
    inheritInfo.renderPass = *_window->defaultRenderPass( );
    inheritInfo.framebuffer = *_window->currentFramebuffer( );

    secondaryCmd->beginSimple( vk::CommandBufferUsageFlagBits::eOneTimeSubmit | vk::CommandBufferUsageFlagBits::eRenderPassContinue, &inheritInfo );
    secondaryCmd->bindGraphicsPipeline( pipeline );

    secondaryCmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descSet }, nullptr );

    secondaryCmd->setViewportScissors( _window->getExtent( ) );
    secondaryCmd->draw( 4, 1, 0, 0 );

    secondaryCmd->end( );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass( 
      _window->defaultRenderPass( ), 
      _window->currentFramebuffer( ), 
      rect, clearValues, vk::SubpassContents::eSecondaryCommandBuffers
    );

    /*cmd->bindGraphicsPipeline( pipeline );

    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descSet }, nullptr );

    cmd->setViewportScissors( _window->getExtent( ) );
    cmd->draw( 4, 1, 0, 0 );*/

    cmd->executeCommands( { secondaryCmd } );

    cmd->endRenderPass( );

    _window->frameReady( );
    //_window->requestUpdate( );
  }

private:
  VulkanWindow *_window;
  float _green = 0;

  std::shared_ptr< DescriptorSetLayout > descSetLayout;
  std::shared_ptr< DescriptorSet > descSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr<Texture2D> tex;

  CommandBufferPtr secondaryCmd;
};

class CustomVkWindow : public VulkanWindow
{
public:
  VulkanWindowRenderer* createRenderer( void ) override
  {
    return new CustomRenderer( this );
  }
};

int main( void )
{
  std::shared_ptr<Instance> instance;

  // Create instance
  vk::ApplicationInfo appInfo(
    "App Name",
    VK_MAKE_VERSION( 1, 0, 0 ),
    "FooEngine",
    VK_MAKE_VERSION( 1, 0, 0 ),
    VK_API_VERSION_1_0
  );

  std::vector<const char*> layers =
  {
    "VK_LAYER_LUNARG_standard_validation",
  };
  std::vector<const char*> extensions =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
    "VK_KHR_win32_surface", // OS specific surface extension
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };

  instance = Instance::create( vk::InstanceCreateInfo(
    { },
    &appInfo,
    layers.size( ),
    layers.data( ),
    extensions.size( ),
    extensions.data( )
  ) );

  CustomVkWindow w;
  w.setVulkanInstance( instance );
  w.resize( 500, 500 );

  w.show( );

  auto res = w.supportedSampleCounts( );
  return 0;
}