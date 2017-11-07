#include <lava/lava.h>
using namespace lava;

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  struct
  {
    glm::mat4 mvp;
  } uboShadow;
  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;
  struct
  {
    glm::vec3 lightPos;
  } uboFS;
  std::shared_ptr<CommandPool> commandPool;

  std::shared_ptr<UniformBuffer> uniformShadow;
  std::shared_ptr<UniformBuffer> uniformMVP;
  std::shared_ptr<UniformBuffer> uniformLight;

  std::shared_ptr<lava::extras::CustomFBO> fbo;

  void prepareOffscreenRenderPass( uint32_t width, uint32_t height )  // 2048, 2048
  {
    fbo = std::make_shared<lava::extras::CustomFBO>( _device, width, height );
    fbo->addColorDepthAttachment( vk::Format::eD16Unorm );
    fbo->build( );

    // Create sampler to sample from the color attachments
    vk::SamplerCreateInfo sampler;
    sampler.magFilter = vk::Filter::eLinear;
    sampler.minFilter = vk::Filter::eLinear;
    sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.minLod = 0.0f;
    sampler.maxLod = 1.0f;
    sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;

    vk::Device device = static_cast< vk::Device >( *_device );
    device.destroySampler( fbo->colorSampler );

    fbo->colorSampler = device.createSampler( sampler );
  }

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    prepareOffscreenRenderPass( width * 2, height * 2 );
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

    uniformShadow = std::make_shared<UniformBuffer>( _device, sizeof( uboShadow ) );
    uniformMVP = std::make_shared<UniformBuffer>( _device, sizeof( uboVS ) );
    uniformLight = std::make_shared<UniformBuffer>( _device, sizeof( uboFS ) );
  }
  virtual void doPaint( void ) override
  {
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    float greenValue = sin( time ) / 2.0f + 0.5f;

    std::array<float, 4> ccv = { 0.0f, greenValue, 0.0f, 1.0f };
    commandBuffer->beginSimple( );
    commandBuffer->beginRenderPass( 
      _renderPass, 
      _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ),
        { 
          vk::ClearValue( ccv ),
          vk::ClearValue( vk::ClearDepthStencilValue( 1.0f, 0 ) )
        },
      vk::SubpassContents::eInline );
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    commandBuffer->endRenderPass( );
    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo {
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
    VulkanApp* app = new MyApp( "Shadow Mapping", 800, 600 );

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