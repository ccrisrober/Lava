#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include <functional>

/*
 * In addition to allocating descriptor sets and binding them to a command buffer, 
 *  an application can record descriptor updates into the command buffer.
*/

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

  void swapTexture( std::shared_ptr<Texture2D> tex, std::shared_ptr<CommandBuffer> cmd )
  {
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( _descriptorSet, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    cmd->pushDescriptorSetKHR( vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, wdss );
  }

  std::shared_ptr<CommandPool> commandPool;
  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<Texture2D> tex1 = std::make_shared<Texture2D>( _device, 
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "chesterfieldDiffuseMap.png" ), 
      commandPool, _graphicsQueue, vk::Format::eR8G8B8A8Unorm );
    std::shared_ptr<Texture2D> tex2 = std::make_shared<Texture2D>( _device, 
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "chesterfieldNormalMap.png" ), 
      commandPool, _graphicsQueue, vk::Format::eR8G8B8A8Unorm );

    cpp = new CustomPingPong<std::shared_ptr<Texture2D>>( tex1, tex2 );

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs = {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    // Specify that descriptor set layout will be for push descriptors
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
      _device->createDescriptorSetLayout( dslbs, 
        vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR );

    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );


    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( {}, 1, { { vk::DescriptorType::eCombinedImageSampler, 1 } } );

    // Init descriptor set
    _descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    // init pipeline
    std::shared_ptr<PipelineCache> pipelineCache = 
      _device->createPipelineCache( 0, nullptr );

    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_frag.spv" ),
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
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );
    if ( i == 0 )
    {
      i = 50;
      cpp->swap( );
      swapTexture( cpp->first( ), commandBuffer );
    }
    --i;

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 }, 
        _defaultFramebuffer->getExtent( ) ),
    { vk::ClearValue( ccv ), vk::ClearValue( 
      vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( _pipeline );
    //commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
    //  _pipelineLayout, 0, { _descriptorSet }, nullptr );

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
    VulkanApp* app = new MyApp( "Push Descriptors", 800, 600 );

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