#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include <functional>

template <class T>
class CustomPingPong
{
public:
  CustomPingPong( const T & elem1, const T & elem2 );
  void swap( void );
  void swap( std::function<void()> cb );
  T first( void ) const;
  T last( void ) const;
protected:
  T _elem1;
  T _elem2;
};

template<typename T>
CustomPingPong<T>::CustomPingPong(const T & elem1, const T & elem2)
: _elem1(std::move(elem1))
, _elem2(std::move(elem2))
{
}
template<typename T>
void CustomPingPong<T>::swap( )
{
  std::swap( _elem1, _elem2 );
}
template<typename T>
void CustomPingPong<T>::swap(std::function<void()> cb)
{
  std::swap(_elem1, _elem2);
  if (cb)
  {
    cb();
  }
}
template<typename T>
T CustomPingPong<T>::first() const
{
  return _elem1;
}
template<typename T>
T CustomPingPong<T>::last() const
{
  return _elem2;
}


class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Pipeline> _pipeline;
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<vk::ImageView> _textureImageView;
  std::shared_ptr<Sampler> _textureSampler;
  std::shared_ptr<DescriptorSet> _descriptorSet;
  std::shared_ptr<Texture2D> tex1;
  std::shared_ptr<Texture2D> tex2;

  CustomPingPong<std::shared_ptr<Texture2D>> *cpp;

  virtual ~MyApp( void )
  {
    delete cpp;
  }

  void swapTexture( std::shared_ptr<Texture2D> tex )
  {
    std::vector<WriteDescriptorSet> wdss;

    WriteDescriptorSet w( _descriptorSet, 0, 0, vk::DescriptorType::eCombinedImageSampler, 1, 
      DescriptorImageInfo( 
        vk::ImageLayout::eGeneral, 
        std::make_shared<vk::ImageView>( tex->view ), 
        std::make_shared<vk::Sampler>( tex->sampler )
      ), nullptr
    );
    wdss.push_back( w );
    _device->updateDescriptorSets( wdss, {} );
  }

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<Texture2D> tex1 = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "chesterfieldDiffuseMap.png" ), commandPool, _graphicsQueue );
    std::shared_ptr<Texture2D> tex2 = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "chesterfieldNormalMap.png" ), commandPool, _graphicsQueue );

    cpp = new CustomPingPong<std::shared_ptr<Texture2D>>( tex1, tex2 );

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs;
    DescriptorSetLayoutBinding mvpDescriptor( 0, vk::DescriptorType::eCombinedImageSampler, 
      vk::ShaderStageFlagBits::eFragment );
    dslbs.push_back( mvpDescriptor );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );


    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 1, { { vk::DescriptorType::eCombinedImageSampler, 1 } } );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    swapTexture( cpp->first( ) );

    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule =
      _device->createShaderModule( 
        LAVA_EXAMPLES_SPV_ROUTE + std::string("fullquad_vert.spv"), 
        vk::ShaderStageFlagBits::eVertex
      );
    std::shared_ptr<ShaderModule> fragmentShaderModule = 
      _device->createShaderModule( 
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_frag.spv" ), 
        vk::ShaderStageFlagBits::eFragment
      );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = 
      _device->createPipelineCache( 0, nullptr );
    PipelineShaderStageCreateInfo vertexStage( 
      vk::ShaderStageFlagBits::eVertex, vertexShaderModule );
    PipelineShaderStageCreateInfo fragmentStage( 
      vk::ShaderStageFlagBits::eFragment, fragmentShaderModule );
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
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG 
      | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, 
      vk::DynamicState::eScissor } );

    _pipeline = _device->createGraphicsPipeline( pipelineCache, {}, 
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, _renderPass );
  }
  void doPaint( void ) override
  {
    static int i = 0;
    if ( ++i == 10 )
    {
      i = 0;
      cpp->swap( );
      swapTexture( cpp->first( ));
    }
    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 }, 
        _defaultFramebuffer->getExtent( ) ),
    { vk::ClearValue( ccv ), vk::ClearValue( 
      vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( _pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      _pipelineLayout, 0, { _descriptorSet }, nullptr );

    commandBuffer->setViewportScissors( 
      _defaultFramebuffer->getExtent( ).width,
      _defaultFramebuffer->getExtent( ).height
    );
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
    VulkanApp* app = new MyApp( "Swap Texture", 800, 600 );

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