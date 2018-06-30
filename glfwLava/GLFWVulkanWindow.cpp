#include "GLFWVulkanWindow.h"

#include "Engine.h"

#include <thread>

namespace lava
{
  std::thread::id main_thread_id = std::this_thread::get_id( );
  /*void GLFWVulkanWindow::OnWindowResized( GLFWwindow * window, int width, int height )
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
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
      _gfxQueueFamilyIdx );


    cmd = _cmdPool->allocateCommandBuffer( );



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
  }*/

  GLFWVulkanWindow::GLFWVulkanWindow( int width, int height, 
    const std::string & title, bool enableLayers )
    : _initialized( false )
  {
    // GLFW initialization
    glfwInit( );

    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

    window = glfwCreateWindow( width, height, title.c_str( ), nullptr, nullptr );

    glfwSetWindowUserPointer( window, this );

    glfwSetKeyCallback( window, [ ]( GLFWwindow* w, int key, int, int action, int )
    {
      static_cast<GLFWVulkanWindow*>( glfwGetWindowUserPointer( w ) )
        ->keyEvent( key, action );
    } );
    glfwSetCursorPosCallback( window, [ ]( GLFWwindow* w,
      double xpos, double ypos )
    {
      static_cast< GLFWVulkanWindow* >( glfwGetWindowUserPointer( w ) )
        ->mouseEvent( xpos, ypos );
    } );
    glfwSetScrollCallback( window, [ ]( GLFWwindow* w,
      double xoffset, double yoffset )
    {
      static_cast< GLFWVulkanWindow* >( glfwGetWindowUserPointer( w ) )
        ->scrollEvent( xoffset, yoffset );
    } );
    //glfwSetWindowSizeCallback( window, GLFWVulkanWindow::OnWindowResized );

    // Vulkan initialization
    lava::Engine::CreateInfo ci;
    ci.appInfo = "GLFWRender";
    ci.enableValidationLayers = enableLayers;

    unsigned int glfw_exts_count;
    const char** glfw_exts = glfwGetRequiredInstanceExtensions( &glfw_exts_count );
    for ( unsigned int i = 0; i < glfw_exts_count; ++i )
    {
      ci.requiredInstanceExtensions.insert( std::string( glfw_exts[ i ] ) );
    }
    ci.requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    Engine e( ci );
    _instance = lava::Instance::createFromVkInstance( e.GetVkInstance( ) );

    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    assert( _instance->getPhysicalDeviceCount( ) != 0 );
    _physicalDevice = _instance->getPhysicalDevice( 0 );
  }
  GLFWVulkanWindow::~GLFWVulkanWindow( void )
  {
    cleanupVulkan( );
  }

  bool GLFWVulkanWindow::checkDeviceLost( vk::Result err )
  {
    if ( err == vk::Result::eErrorDeviceLost )
    {
      std::cerr << "Device lost" << std::endl;
      /*if ( renderer )
      {
      renderer->logicalDeviceLost( );
      }
      qWarning( "Releasing all resources due to device lost" );
      releaseSwapchain( );
      reset( );
      qWarning( "Restarting" );
      ensureStarted( );*/
      return true;
    }
    return false;
  }
  void GLFWVulkanWindow::addReadback( void )
  {
    /*std::shared_ptr< lava::Image > frameGrabImage =
    _device->createImage( vk::ImageCreateFlagBits( ), vk::ImageType::e2D,
    vk::Format::eR8G8B8A8Unorm,
    vk::Extent3D(
    _frameGrabTargetImage.width( ), _frameGrabTargetImage.height( ), 1
    ), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eLinear,
    vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode( ), { },
    vk::ImageLayout::ePreinitialized, nullptr );

    auto cmd = cmds[ imageIdx ];*/
  }
  void GLFWVulkanWindow::finishBlockingReadback( void )
  {
    // TODO
  }
  void GLFWVulkanWindow::getSurfaceFormats( void )
  {
    auto surfaceFormats = _physicalDevice->getSurfaceFormats( surface );
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
  }
  void GLFWVulkanWindow::createQueues( void )
  {
    // Search for a graphics queue and a present queue in the array of 
    //    queue families, try to find one that supports both
    std::vector<vk::QueueFamilyProperties> queueFamilyIndices =
      _physicalDevice->getQueueFamilyProperties( );
    assert( !queueFamilyIndices.empty( ) );

    _gfxQueueFamilyIdx = uint32_t( -1 );
    _presQueueFamilyIdx = uint32_t( -1 );

    VkBool32 presentSupport = VK_FALSE;
    for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
    {
      presentSupport = _physicalDevice->supportSurfaceKHR( i, surface );

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
        break;
      }
    }

    if ( _gfxQueueFamilyIdx != uint32_t( -1 ) )
    {
      _presQueueFamilyIdx = _gfxQueueFamilyIdx;
    }
    else
    {
      std::cerr << "No queue with graphics + present; trying separate queues"
        << std::endl;
      for ( uint32_t i = 0, l = queueFamilyIndices.size( ); i < l; ++i )
      {
        if ( _gfxQueueFamilyIdx == uint32_t( -1 ) &&
          ( queueFamilyIndices[ i ].queueFlags & vk::QueueFlagBits::eGraphics ) )
        {
          _gfxQueueFamilyIdx = i;
        }

        presentSupport = _physicalDevice->supportSurfaceKHR( i, surface );
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
  }

  void GLFWVulkanWindow::show( void )
  {
    if ( !_initialized )
    {
      initVulkan( );
    }

    while ( !glfwWindowShouldClose( window ) )
    {
      beginFrame( );

      // Very crude method to prevent your GPU from overheating.
      std::this_thread::sleep_for( std::chrono::milliseconds( 16 ) );

      requestUpdate( );
    }
  }

  static struct {
    vk::SampleCountFlagBits mask;
    int count;
  } sampleCounts[ ] = {
    // keep this sorted by 'count'
    { vk::SampleCountFlagBits::e1, 1 },
    { vk::SampleCountFlagBits::e2, 2 },
    { vk::SampleCountFlagBits::e4, 4 },
    { vk::SampleCountFlagBits::e8, 8 },
    { vk::SampleCountFlagBits::e16, 16 },
    { vk::SampleCountFlagBits::e32, 32 },
    { vk::SampleCountFlagBits::e64, 64 }
  };

  void GLFWVulkanWindow::setSampleCountFlagBits( int sampleCount_ )
  {
    for ( uint32_t i = 0, l = sizeof( sampleCounts ) /
      sizeof( sampleCounts[ 0 ] ); i < l; ++i )
    {
      if ( sampleCounts[ i ].count == sampleCount_ )
      {
        sampleCount = sampleCounts[ i ].mask;
        return;
      }
    }
  }
  std::vector<int> GLFWVulkanWindow::supportedSampleCounts( void )
  {
    auto limits = physicalDevice( )->getDeviceProperties( ).limits;
    vk::SampleCountFlags color = limits.framebufferColorSampleCounts;
    vk::SampleCountFlags depth = limits.framebufferDepthSampleCounts;
    vk::SampleCountFlags stencil = limits.framebufferStencilSampleCounts;

    std::vector< int > result;

    for ( uint32_t i = 0, l = sizeof( sampleCounts ) / sizeof( sampleCounts[ 0 ] );
      i < l; ++i )
    {
      if ( ( color & sampleCounts[ i ].mask )
        && ( depth & sampleCounts[ i ].mask )
        && ( stencil & sampleCounts[ i ].mask ) )
      {
        result.push_back( sampleCounts[ i ].count );
      }
    }
    return result;
  }
  void GLFWVulkanWindow::beginFrame( void )
  {
    if ( !_dfbFramebuffer || !_dfbFramebuffer->swapchain( ) || _framePending ) return;

    vk::Extent2D extent = _dfbFramebuffer->extent( );
    if ( _dfbFramebuffer->extent( ) != swapchainImageSize( ) )
    {
      recreateSwapchain( );
      if ( !_dfbFramebuffer->swapchain( ) )
      {
        return;
      }
    }

    // move on to next swapchain image
    auto res = _dfbFramebuffer->swapchain( )->acquireNextImage( );

    if ( res.result == vk::Result::eErrorOutOfDateKHR )
    {
      std::cerr << "Swapchain out of date" << std::endl;
      // swapchain is out of date (e.g. the window was resized) and
      // must be recreated:
      recreateSwapchain( );
      return;
    }
    else if ( res.result != vk::Result::eSuccess &&
      res.result != vk::Result::eSuboptimalKHR )
    {
      std::cerr << "Swapchain SUBOPTIMAL" << std::endl;
      // swapchain is not as optimal as it could be, but the platform's
      // presentation engine will still present the image correctly.
      throw std::runtime_error( "Failed to acquire swapchain image" );
    }

    imageIdx = res.value;
    cmds[ imageIdx ] = _cmdPool->allocateCommandBuffer( );

    auto cmd = cmds[ imageIdx ];
    cmd->begin( );

    if ( _frameGrabbing )
    {
      // TODO _frameGrabTargetImage = QImage( size( ), QImage::Format_RGBA8888 );
    }

    if ( renderer )
    {
      _framePending = true;
      renderer->nextFrame( );
      // renderer call to frameReady( ) who calls endFrame( )
    }
    else
    {
      std::array<vk::ClearValue, 2 > clearValues;
      std::array<float, 4> ccv = { 1.0f, 1.0f, 1.0f, 1.0f };

      clearValues[ 0 ].color = vk::ClearColorValue( ccv );
      clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

      cmd->beginRenderPass( _dfbFramebuffer->renderPass( ),
        _dfbFramebuffer->framebuffer( imageIdx ),
        vk::Rect2D( { 0, 0 }, swapchainImageSize( ) ), clearValues,
        vk::SubpassContents::eInline );

      cmd->endRenderPass( );

      endFrame( );
    }
  }
  void GLFWVulkanWindow::endFrame( void )
  {
    // RENDER
    {
      try
      {
        auto cmd = cmds[ imageIdx ];

        if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx && !_frameGrabbing )
        {
          // Add swapchain image release to the command buffer that will be
          // submitted to the graphics queue.
          lava::ImageMemoryBarrier presTrans(
            vk::AccessFlagBits( ), vk::AccessFlagBits::eColorAttachmentWrite,
            vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::ePresentSrcKHR,
            _gfxQueueFamilyIdx, _presQueueFamilyIdx,
            nullptr, // TODO: IMAGE
            vk::ImageSubresourceRange( )
            .setAspectMask( vk::ImageAspectFlagBits::eColor )
            .setLevelCount( 1 )
            .setLayerCount( 1 )
          );

          cmd->pipelineBarrier( vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlagBits( ),
            { }, { }, presTrans );
        }

        // When grabbing a frame, add a readback at the end and skip presenting.
        if ( _frameGrabbing )
        {
          addReadback( );
        }

        cmd->end( );

        vk::Result res = _gfxQueue->submit( SubmitInfo{
          _dfbFramebuffer->swapchain( )->getPresentCompleteSemaphores( )[ imageIdx ],
          { vk::PipelineStageFlagBits::eColorAttachmentOutput },
          cmd,
          _renderComplete
        } );
      }
      catch ( std::exception e )
      {
        std::cerr << e.what( ) << std::endl;
      }
    }

    // block and then bail out when grabbing
    if ( _frameGrabbing )
    {
      finishBlockingReadback( );
      _frameGrabbing = false;
      // Leave frame.imageAcquired set to true.
      // Don't change currentFrame.
      //emit frameGrabbed( frameGrabTargetImage );
      return;
    }

    if ( _gfxQueueFamilyIdx != _presQueueFamilyIdx )
    {
      /*// Submit the swapchain image acquire to the present queue.
      submitInfo.pWaitSemaphores = &frame.drawSem;
      submitInfo.pSignalSemaphores = &frame.presTransSem;
      submitInfo.pCommandBuffers = &image.presTransCmdBuf; // must be USAGE_SIMULTANEOUS
      err = devFuncs->vkQueueSubmit( presQueue, 1, &submitInfo, VK_NULL_HANDLE );
      if ( err != vk::Result::eSuccess )
      {
      if ( !checkDeviceLost( err ) )
      {
      qWarning( "QVulkanWindow: Failed to submit to present queue: %d", err );
      }
      return;
      }*/
    }





    vk::Result presentResult = vk::Result::eSuccess;
    try
    {
      auto waitSemaphores = { _renderComplete };
      auto swapchains = { _dfbFramebuffer->swapchain( ) };
      std::vector<uint32_t> imageIndices = { imageIdx };
      vk::Queue _queue = *_presQueue;
      {
        std::vector<vk::Semaphore> waitSemaphoreData;
        waitSemaphoreData.reserve( waitSemaphores.size( ) );
        for ( auto const& s : waitSemaphores )
        {
          waitSemaphoreData.push_back( *s );
        }

        std::vector<vk::SwapchainKHR> swapchainData;
        swapchainData.reserve( swapchains.size( ) );
        for ( auto const& s : swapchains )
        {
          swapchainData.push_back( static_cast< vk::SwapchainKHR >( *s ) );
        }

        presentResult = _queue.presentKHR( vk::PresentInfoKHR(
          waitSemaphoreData.size( ), waitSemaphoreData.data( ),
          swapchainData.size( ), swapchainData.data( ),
          imageIndices.data( ) ) );
        //return results;
      }
      //_presQueue->present( { signalSemaphore } )
    }
    catch ( vk::OutOfDateKHRError )
    {
      recreateSwapchain( );
      presentResult = vk::Result::eSuccess;
    }
    catch ( std::exception e )
    {
      std::cout << e.what( ) << std::endl;
    }
    if ( presentResult == vk::Result::eSuboptimalKHR )
    {
      recreateSwapchain( );
    }
    else if ( presentResult != vk::Result::eSuccess )
    {
      throw "Failed to present swap chain image.";
    }
    
    // TODO: REVISAR
    _presQueue->waitIdle( );
  }
  void GLFWVulkanWindow::frameReady( void )
  {
    if ( main_thread_id != std::this_thread::get_id( ) )
    {
      std::cerr << "You only can called this in main thread" << std::endl;
      return;
    }
    // TODO: Check only called by main thread std::this_thread::
    if ( !_framePending )
    {
      throw "framePending() called without calling nextFrame( )";
    }
    _framePending = false;

    endFrame( );
  }
  bool GLFWVulkanWindow::setupPipelineCache( void )
  {
    _pipelineCache = nullptr;
    return false;
  }
  void GLFWVulkanWindow::OnWindowResized( GLFWwindow * window, int width, int height )
  {
  }
  void GLFWVulkanWindow::initVulkan( void )
  {
    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    if ( !renderer )
    {
      renderer = createRenderer( );
    }

    if ( !_physicalDevice )
    {
      LAVA_RUNTIME_ERROR( "Failed to find a device with presentation support" );
    }

    {
      VkSurfaceKHR c_surface;

      VkResult result = glfwCreateWindowSurface(
        VkInstance( static_cast< vk::Instance >( *_instance ) ), window, nullptr,
        &c_surface );

      if ( result != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create window surface!" );
      }

      surface = std::make_shared< lava::Surface >( _instance,
        vk::SurfaceKHR( c_surface ), true );
    }

    getSurfaceFormats( );

    createQueues( );

    VkBool32 validDepthFormat = lava::utils::getSupportedDepthFormat(
      _physicalDevice, _dsFormat );
    assert( validDepthFormat );

    _dfbFramebuffer = new DefaultFramebuffer( _device, surface,
      swapchainImageSize( ), _dsFormat, sampleCount );

    _cmdPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits( ),
      //vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
      _gfxQueueFamilyIdx );

    size_t numImages = _dfbFramebuffer->swapchain( )->count( );
    cmds.reserve( numImages );
    for ( size_t i = 0; i < numImages; ++i )
    {
      cmds.push_back( _cmdPool->allocateCommandBuffer( ) );
    }

    setupPipelineCache( );

    if ( renderer )
    {
      renderer->initResources( );
    }

    _renderComplete = _device->createSemaphore( );

    _initialized = true;
  }
  void GLFWVulkanWindow::cleanupVulkan( void )
  {
    if ( !_initialized ) return;

    _device->waitIdle( );

    if ( renderer )
    {
      renderer->releaseResources( );
      _device->waitIdle( );

      delete renderer;

      renderer = nullptr;
    }

    cmds.clear( );

    delete _dfbFramebuffer;
    _dfbFramebuffer = nullptr;

    if ( _renderComplete )
    {
      _renderComplete.reset( );
      _renderComplete = nullptr;
    }

    _initialized = false;
  }
  void GLFWVulkanWindow::recreateSwapchain( void )
  {
    _dfbFramebuffer->recreate( );
  }
  GLFWVulkanWindowRenderer * GLFWVulkanWindow::createRenderer( void )
  {
    return nullptr;
  }
}