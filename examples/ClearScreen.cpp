#include <lava/lava.h>
using namespace lava;

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<CommandPool> commandPool;
  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
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
    VulkanApp* app = new MyApp( "MyApp", 800, 600 );

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