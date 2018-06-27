#include "GLFWVulkanWindow.h"

#include "Engine.h"

namespace lava
{
  void GLFWVulkanWindow::OnWindowResized( GLFWwindow * window, int width, int height )
  {
  }
  void GLFWVulkanWindow::InitVulkan( bool enableLayers )
  {
    lava::Engine::CreateInfo ci;
    ci.appInfo = "QtRender";
    ci.enableValidationLayers = enableLayers;

    unsigned int glfw_extensions_count;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extensions_count );

    for ( unsigned int i = 0; i < glfw_extensions_count; ++i )
    {
      ci.requiredInstanceExtensions.insert( std::string( glfw_extensions[ i ] ) );
    }

    ci.requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    Engine e( ci );
    _instance = lava::Instance::createFromVkInstance( e.GetVkInstance( ) );



    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    if ( !renderer )
    {
      renderer = createRenderer( );
    }

    assert( _instance->getPhysicalDeviceCount( ) != 0 );
    _physicalDevice = _instance->getPhysicalDevice( 0 );

    if ( !_physicalDevice )
    {
      LAVA_RUNTIME_ERROR( "Failed to find a device with presentation support" );
    }

    createSurface( );

    auto surfaceFormats = _physicalDevice->getSurfaceFormats( _surface );
    assert( !surfaceFormats.empty( ) );
    uint32_t numFormats = surfaceFormats.size( );

    bool gamma = false;
    // If there is no preferred format, use standard RGBA
    if ( ( numFormats == 1 )
      && ( surfaceFormats[ 0 ].format == vk::Format::eUndefined ) )
    {
      _surfaceFormat.format = gamma ? vk::Format::eR8G8B8A8Srgb : vk::Format::eB8G8R8A8Unorm;

      _surfaceFormat.colorSpace = surfaceFormats[ 0 ].colorSpace;
    }
    else
    {
      bool foundFormat = false;

      std::vector<vk::Format> wantedFormatsUNORM =
      {
        vk::Format::eR8G8B8A8Unorm,
        vk::Format::eA8B8G8R8UnormPack32,
        vk::Format::eB8G8R8Unorm
      };

      std::vector<vk::Format> wantedFormatsSRGB =
      {
        vk::Format::eB8G8R8A8Srgb,
        vk::Format::eA8B8G8R8SrgbPack32,
        vk::Format::eB8G8R8Srgb
      };

      std::vector<vk::Format> wantedFormats = gamma ? wantedFormatsSRGB : wantedFormatsUNORM;

      for ( const auto& wantedFormat : wantedFormats )
      {
        for ( const auto& surfFormat : surfaceFormats )
        {
          if ( surfFormat.format == wantedFormat )
          {
            _surfaceFormat.format = surfFormat.format;
            _surfaceFormat.colorSpace = surfFormat.colorSpace;

            foundFormat = true;
            break;
          }
        }
        if ( foundFormat )
        {
          break;
        }
      }

      wantedFormatsSRGB.clear( );
      wantedFormatsUNORM.clear( );
      wantedFormats.clear( );

      // If we haven't found anything, fall back to first available
      if ( !foundFormat )
      {
        _surfaceFormat.format = surfaceFormats[ 0 ].format;
        _surfaceFormat.colorSpace = surfaceFormats[ 0 ].colorSpace;

        if ( gamma )
        {
          throw new std::runtime_error( R"(Cannot find a valid sRGB format for a render window surface, falling back to a default format.)" );
        }
      }
    }


    VkBool32 validDepthFormat = lava::utils::getSupportedDepthFormat(
      _physicalDevice, _dsFormat );
    assert( validDepthFormat );

    // Search for a graphics queue and a present queue in the array of 
    //    queue families, try to find one that supports both

    std::vector<vk::QueueFamilyProperties> queueFamilyIndices =
      _physicalDevice->getQueueFamilyProperties( );
    assert( !queueFamilyIndices.empty( ) );

    _gfxQueueFamilyIdx = uint32_t( -1 );
    _presQueueFamilyIdx = uint32_t( -1 );

    auto phyDev = static_cast< vk::PhysicalDevice >( *_physicalDevice );
    for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
    {
      VkBool32 presentSupport = phyDev.getSurfaceSupportKHR( i, _surface );

      printf( "queue family %d: flags=0x%x count=%d supportsPresent=%d\n",
        i,
        static_cast< VkQueueFlags >( queueFamilyIndices[ i ].queueFlags ),
        queueFamilyIndices[ i ].queueCount,
        presentSupport == VK_TRUE ? 1 : 0
      );

      if ( _gfxQueueFamilyIdx == uint32_t( -1 ) &&
        ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics )
        && presentSupport )
      {
        _gfxQueueFamilyIdx = i;
        //break;
      }
    }

    if ( _gfxQueueFamilyIdx != uint32_t( -1 ) )
    {
      _presQueueFamilyIdx = _gfxQueueFamilyIdx;
    }
    else
    {
      std::cerr << "No queue with graphics+present; trying separate queues" << std::endl;
      for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
      {
        if ( _gfxQueueFamilyIdx == uint32_t( -1 ) &&
          ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics ) )
        {
          _gfxQueueFamilyIdx = i;
        }

        VkBool32 presentSupport = phyDev.getSurfaceSupportKHR( i, _surface );
        if ( _presQueueFamilyIdx == uint32_t( -1 ) && presentSupport )
        {
          _presQueueFamilyIdx = i;
        }
      }
    }
    if ( _gfxQueueFamilyIdx == uint32_t( -1 ) )
    {
      std::cerr << "ERROR: No graphics queue family found" << std::endl;
      throw;
    }
    if ( _presQueueFamilyIdx == uint32_t( -1 ) )
    {
      std::cerr << "ERROR: No present queue family found" << std::endl;
      throw;
    }

    /*std::vector<uint32_t> queueFamilyIndices =
    _physicalDevice->getGraphicsPresentQueueFamilyIndices( _surface );
    assert( !queueFamilyIndices.empty( ) );
    _gfxQueueFamilyIdx = queueFamilyIndices[ 0 ];*/

    // Create a new device with the VK_KHR_SWAPCHAIN_EXTENSION enabled.
    std::vector<std::string> enabledLayerNames;

    std::vector<std::string> enabledExtensionNames;

    _requestedDeviceExts.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );

    for ( const auto& ext : _requestedDeviceExts )
    {
      if ( _physicalDevice->extensionSupported( ext ) )
      {
        enabledExtensionNames.push_back( ext );
      }
    }

    vk::DeviceQueueCreateInfo dqci;
    dqci.setQueueFamilyIndex( _gfxQueueFamilyIdx );

    const float queuePriority = 0.0f;
    dqci.setQueueCount( 1 );
    dqci.setPQueuePriorities( &queuePriority );
    std::vector< vk::DeviceQueueCreateInfo > queueCreateInfos;
    queueCreateInfos.push_back( dqci );


    if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx )
    {
      vk::DeviceQueueCreateInfo dqci2;
      dqci2.setQueueFamilyIndex( _presQueueFamilyIdx );
      dqci2.setQueueCount( 1 );
      dqci2.setPQueuePriorities( &queuePriority );

      queueCreateInfos.push_back( dqci2 );
    }

    _device = _physicalDevice->createDevice(
      queueCreateInfos,
      enabledLayerNames,
      enabledExtensionNames,
      _physicalDevice->getDeviceFeatures( )
    );

    _gfxQueue = _device->getQueue( _gfxQueueFamilyIdx, 0 );

    if ( _gfxQueueFamilyIdx == _presQueueFamilyIdx )
    {
      _presQueue = _gfxQueue;
    }
    else
    {
      _presQueue = _device->getQueue( _presQueueFamilyIdx, 0 );
    }













    int width, height;
    glfwGetWindowSize( window, &width, &height );
    vk::Extent2D extent( width, height );






    //_swapchain = std::make_shared< SwapChain >( _device, _surface, _presQueueFamilyIdx, extent );
    _dfbFramebuffer = new DefaultFramebuffer( _device, _surface,
      _presQueueFamilyIdx, extent, _dsFormat );

    _cmdPool = _device->createCommandPool(
      /*vk::CommandPoolCreateFlagBits(), //*/vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
      _gfxQueueFamilyIdx );


    cmd = _cmdPool->allocateCommandBuffer( );

    /*if ( !setupRenderPass( ) )
    {
    return;
    }

    _renderComplete = _device->createSemaphore( );

    setupFramebuffer( );*/

    //setupPipelineCache( );


    if ( renderer )
    {
      renderer->initResources( );
    }

    imageAvailableSem = _device->createSemaphore( );
    renderFinishedSem = _device->createSemaphore( );
  }
  void GLFWVulkanWindow::InitWindow( int width, int height, const std::string & title )
  {
    glfwInit( );

    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

    window = glfwCreateWindow( width, height, title.c_str( ), nullptr, nullptr );

    glfwSetWindowUserPointer( window, this );
    glfwSetWindowSizeCallback( window, GLFWVulkanWindow::OnWindowResized );
  }
  void GLFWVulkanWindow::OnWindowResized( int width, int height )
  {
  }
  void GLFWVulkanWindow::createSurface( void )
  {
    VkSurfaceKHR c_surface;

    VkResult result = glfwCreateWindowSurface( 
      VkInstance( static_cast< vk::Instance >( *_instance ) ), window, nullptr, 
      &c_surface );

    if ( result != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create window surface!" );
    }

    _surface = vk::SurfaceKHR( c_surface );
  }
  void GLFWVulkanWindow::recreateSwapchain( void )
  {
    _dfbFramebuffer->recreate( );
  }
  void GLFWVulkanWindow::cleanup( )
  {
    _device->waitIdle( );
    delete _dfbFramebuffer;

    imageAvailableSem.reset( );
    renderFinishedSem.reset( );

    glfwDestroyWindow( window );
    glfwTerminate( );
  }
  GLFWVulkanWindowRenderer * GLFWVulkanWindow::createRenderer( void )
  {
    return nullptr;
  }
  GLFWVulkanWindow::GLFWVulkanWindow( int width, int height, const std::string & title, bool enableLayers )
  {
    InitWindow( width, height, title );
    InitVulkan( enableLayers );
  }
  GLFWVulkanWindow::~GLFWVulkanWindow( void )
  {
    cleanup( );
  }
  void GLFWVulkanWindow::beginFrame( void )
  {
  }
  void GLFWVulkanWindow::update( void )
  {
    glfwPollEvents( );
  }
  void GLFWVulkanWindow::endFrame( void )
  {
    _device->waitIdle( );
  }

  void GLFWVulkanWindow::render( void )
  {
    int width, height;
    glfwGetWindowSize( window, &width, &height );
    vk::Extent2D extent( width, height );
    vk::Extent2D extent2 = _dfbFramebuffer->extent( );
    if ( extent != extent2 )
    {
      recreateSwapchain( );
      return;
    }

    vk::Device device = *_device;
    auto image_index_result = device.acquireNextImageKHR( *_dfbFramebuffer->swapchain( ), UINT64_MAX, *imageAvailableSem, vk::Fence( ) );

    if ( image_index_result.result == vk::Result::eErrorOutOfDateKHR )
    {
      recreateSwapchain( );
      return;
    }
    else if ( image_index_result.result != vk::Result::eSuccess && image_index_result.result != vk::Result::eSuboptimalKHR )
    {
      throw std::runtime_error( "failed to acquire swap chain image!" );
    }

    uint32_t image_index = image_index_result.value;


    drawFrame(
      image_index,
      { *imageAvailableSem },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      { *renderFinishedSem }
    );

    vk::Semaphore signalSem[ ] = { *renderFinishedSem };
    vk::Result presentResult;
    vk::Queue presentQueue = *_presQueue;
    try
    {
      vk::SwapchainKHR sc = *_dfbFramebuffer->swapchain( );
      presentResult = presentQueue.presentKHR(
        vk::PresentInfoKHR( )
        .setWaitSemaphoreCount( 1 )
        .setPWaitSemaphores( signalSem )
        .setSwapchainCount( 1 )
        .setPSwapchains( &sc )
        .setPImageIndices( &image_index ));
    }
    catch ( vk::OutOfDateKHRError )
    {
      recreateSwapchain( );
      presentResult = vk::Result::eSuccess;
    }

    if ( presentResult == vk::Result::eSuboptimalKHR )
    {
      recreateSwapchain( );
    }
    else if ( presentResult != vk::Result::eSuccess )
    {
      throw std::runtime_error( "failed to present swap chain image!" );
    }

    presentQueue.waitIdle( );
  }
  void GLFWVulkanWindow::drawFrame( std::uint32_t imageIdx,
    std::vector<vk::Semaphore> wait_semaphores, 
    std::vector<vk::PipelineStageFlags> wait_stages, 
    std::vector<vk::Semaphore> signal_semaphores )
  {
    cmd->reset( );
    cmd->begin( );


    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;

    float _red = sin( time ) * 0.5f + 0.5f;
    float _blue = cos( time ) * 0.5f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { _red, 0.0f, _blue, 1.0f };

    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::Extent2D extent = _dfbFramebuffer->extent( );

    cmd->beginRenderPass( _dfbFramebuffer->renderPass( ),
      _dfbFramebuffer->framebuffer( imageIdx ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->endRenderPass( );
    cmd->end( );


    vk::CommandBuffer render_command_buffer = *cmd;
    vk::Queue gq = *_gfxQueue;
    gq.submit(
      vk::SubmitInfo( )
      .setWaitSemaphoreCount( static_cast<uint32_t>( wait_semaphores.size( ) ) )
      .setPWaitSemaphores( wait_semaphores.data( ) )
      .setPWaitDstStageMask( wait_stages.data( ) )
      .setCommandBufferCount( 1 )
      .setPCommandBuffers( &render_command_buffer )
      .setSignalSemaphoreCount( static_cast<uint32_t>( signal_semaphores.size( ) ) )
      .setPSignalSemaphores( signal_semaphores.data( ) ),
      nullptr );
  }
}