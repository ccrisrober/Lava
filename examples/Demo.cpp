#include <lava/lava.h>

using namespace lava;

#define VK_USE_PLATFORM_WIN32_KHR

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
  }

  void nextFrame( void ) override
  {
    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(
      currentTime - startTime ).count( ) / 1000.0f;

    _green = sin( time ) / 2.0f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, _green, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil  = vk::ClearDepthStencilValue(  1.0f, 0 );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass( 
      _window->defaultRenderPass( ), 
      _window->currentFramebuffer( ), 
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->endRenderPass( );

    _window->frameReady( );
    //_window->requestUpdate( );
  }

private:
  VulkanWindow *_window;
  float _green = 0;
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
  w.resize( 800, 800 );

  w.show( );

  auto res = w.supportedSampleCounts( );
  return 0;
}