#include "VulkanApp.h"
#include <iostream>
#include <iomanip>

#include "Descriptor.h"

namespace lava
{
  VulkanApp::VulkanApp( const char* title, uint32_t width, uint32_t height )
  {
    _window = std::make_shared< Window >( title, width, height );

    if ( !_window ) throw;

    // Create instance
    vk::ApplicationInfo appInfo(
      "App Name",
      VK_MAKE_VERSION( 1, 0, 0 ),
      "FooEngine",
      VK_MAKE_VERSION( 1, 0, 0 ),
      VK_API_VERSION_1_0
    );

    std::vector< const char* > exts;
    uint32_t extensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions( &extensionCount );

    std::vector<vk::ExtensionProperties> extensions_ =
      vk::enumerateInstanceExtensionProperties( );
#ifndef NDEBUG
    std::vector<const char*> layers =
    {
      "VK_LAYER_LUNARG_standard_validation",
      //"VK_LAYER_LUNARG_api_dump"

    };
    if ( !checkValidationLayerSupport( layers ) ) throw;
    std::vector<const char*> extensions =
    {
      glfwExtensions[ 0 ],	// Surface extension
      glfwExtensions[ 1 ],	// OS specific surface extension
      VK_EXT_DEBUG_REPORT_EXTENSION_NAME
    };
#else
    std::vector<const char*> layers;
    std::vector<const char*> extensions =
    {
      glfwExtensions[ 0 ],	// Surface extension
      glfwExtensions[ 1 ],	// OS specific surface extension
    };
#endif
    std::cout << "available extensions:" << std::endl;
    bool founded = false;
    for ( const auto& extension : extensions_ )
    {
      std::cout << "[Vulkan init] extensions: name="
        << extension.extensionName << ", enabled=";

      founded = false;
      for ( const auto& e : extensions )
      {
        if ( strcmp( e, extension.extensionName ) == 0 )
        {
          founded = true;
          break;
        }
      }
      if ( founded ) std::cout << "1";
      else std::cout << "0";

      std::cout << std::endl;
    }

    {
      std::cout << std::endl << std::endl;

      std::vector<vk::LayerProperties> layersPropList =
        vk::enumerateInstanceLayerProperties( );

      std::cout << "Instance layers: " << layersPropList.size( )
        << " item(s)\n";
      for ( const auto& l : layersPropList )
      {
        founded = false;
        for ( const auto& ll : layers )
        {
          if ( strcmp( ll, l.layerName ) == 0 )
          {
            founded = true;
            break;
          }
        }
        std::cout << " " << std::left << std::setw( 40 ) <<
          l.layerName << " (";
        if ( founded ) std::cout << "1";
        else std::cout << "0";
        std::cout << ")| " << std::setw( 100 ) << l.description
          << std::endl;
      }
    }
    instance = Instance::create( vk::InstanceCreateInfo(
      vk::InstanceCreateFlags( ),
      &appInfo,
      layers.size( ),
      layers.data( ),
      extensions.size( ),
      extensions.data( )
    ) );
#ifndef NDEBUG
    vk::DebugReportFlagsEXT debugFlags = vk::DebugReportFlagBitsEXT::eError |
      vk::DebugReportFlagBitsEXT::eWarning |
      vk::DebugReportFlagBitsEXT::ePerformanceWarning;

    instance->createDebugReportCallback( vk::DebugReportCallbackCreateInfoEXT(
      debugFlags, &lava::debugMsgCallback ) );
#endif

    // Find a physical device with presentation support
    assert( instance->getPhysicalDeviceCount( ) != 0 );
    VkInstance inst = VkInstance( vk::Instance( *instance ) );
    for ( size_t i = 0; i < instance->getPhysicalDeviceCount( ); ++i )
    {
      VkPhysicalDevice pd( vk::PhysicalDevice( *instance->getPhysicalDevice( i ) ) );
      if ( glfwGetPhysicalDevicePresentationSupport( inst, pd, 0 ) )
      {
        _physicalDevice = instance->getPhysicalDevice( i );
        break;
      }
    }
    if ( !_physicalDevice )
    {
      throw std::runtime_error( "Failed to find a device with presentation support" );
    }

    initCapabilities( );

    _surface = instance->createSurfaceKHR( _window->getWindow( ) );

    std::vector<vk::SurfaceFormatKHR> surfaceFormats =
      _physicalDevice->getSurfaceFormats( _surface );
    assert( !surfaceFormats.empty( ) );
    uint32_t numFormats = surfaceFormats.size( );

    bool gamma = false;
    // If there is no preferred format, use standard RGBA
    if ( ( numFormats == 1 )
      && ( surfaceFormats[ 0 ].format == vk::Format::eUndefined ) )
    {
      if ( gamma )
      {
        _colorFormat = vk::Format::eR8G8B8A8Srgb;
      }
      else
      {
        _colorFormat = vk::Format::eB8G8R8A8Unorm;
      }

      _colorSpace = surfaceFormats[0].colorSpace;
    }
    else
    {
      bool foundFormat = false;

      std::vector<vk::Format> wantedFormatsUNORM =
      {
        vk::Format::eR8G8B8A8Unorm,
        vk::Format::eB8G8R8A8Unorm,
        vk::Format::eA8B8G8R8UnormPack32,
        vk::Format::eA8B8G8R8UnormPack32,
        vk::Format::eR8G8B8Unorm,
        vk::Format::eB8G8R8Unorm
      };

      std::vector<vk::Format> wantedFormatsSRGB =
      {
        vk::Format::eR8G8B8A8Srgb,
        vk::Format::eB8G8R8A8Srgb,
        vk::Format::eA8B8G8R8SrgbPack32,
        vk::Format::eA8B8G8R8SrgbPack32,
        vk::Format::eR8G8B8Srgb,
        vk::Format::eB8G8R8Srgb
      };

      std::vector<vk::Format> wantedFormats;
      if ( gamma )
      {
        wantedFormats = wantedFormatsSRGB;
      }
      else
      {
        wantedFormats = wantedFormatsUNORM;
      }

      for ( const auto& wantedFormat : wantedFormats )
      {
        for ( const auto& surfFormat : surfaceFormats )
        {
          if ( surfFormat.format == wantedFormat )
          {
            _colorFormat = surfFormat.format;
            _colorSpace = surfFormat.colorSpace;

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
        _colorFormat = surfaceFormats[ 0 ].format;
        //_colorSpace = surfaceFormats[0].colorSpace;

        if ( gamma )
        {
          throw new std::runtime_error( R"(Cannot find a valid sRGB format for a render window surface, falling back to a default format.)" );
        }
      }
    }
    _depthFormat = vk::Format::eD24UnormS8Uint;


    // Search for a graphics queue and a present queue in the array of 
    //    queue families, try to find one that supports both
    std::vector<uint32_t> queueFamilyIndices =
      _physicalDevice->getGraphicsPresentQueueFamilyIndices( _surface );
    assert( !queueFamilyIndices.empty( ) );
    _queueFamilyIndex = queueFamilyIndices[ 0 ];

    // Create a new device with the VK_KHR_SWAPCHAIN_EXTENSION enabled.
    std::vector<std::string> enabledLayerNames;
    std::vector<std::string> enabledExtensionNames;
    enabledExtensionNames.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    vk::DeviceQueueCreateInfo dqci;
    dqci.setQueueFamilyIndex( _queueFamilyIndex );
    const float queuePriority = 1.0f;
    dqci.setQueueCount( 1 );
    dqci.setPQueuePriorities( &queuePriority );
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.push_back( dqci );

    _device = _physicalDevice->createDevice( 
      queueCreateInfos, 
      enabledLayerNames,
      enabledExtensionNames,
      _physicalDevice->getDeviceFeatures( )
    );

    _graphicsQueue = _device->getQueue( _queueFamilyIndex, 0 );

    // init render pass
    vk::AttachmentReference colorReference( 0,
      vk::ImageLayout::eColorAttachmentOptimal );
    vk::AttachmentReference depthReference( 1,
      vk::ImageLayout::eDepthStencilAttachmentOptimal );

    vk::SubpassDescription sd(
      vk::SubpassDescriptionFlags( ),
      vk::PipelineBindPoint::eGraphics,
      0, nullptr,           // input attachments ( count, data )
      1, &colorReference,   // color attachments ( count, data )
      nullptr,              // resolve attachments ( data )
      &depthReference,      // depth attachment ( data )
      0, nullptr            // preserve attachments ( count, data )
    );

    _renderPass = _device->createRenderPass(
    {
      vk::AttachmentDescription( // attachment 0
        {}, _colorFormat, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, // color
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, // stencil
        vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
      ),
      vk::AttachmentDescription( // attachment 1
        {}, _depthFormat, vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, // depth
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, // stencil
        vk::ImageLayout::eUndefined,vk::ImageLayout::eDepthStencilAttachmentOptimal
      )
    }, sd, {}
    );

    _renderComplete = _device->createSemaphore( );

    // create Framebuffer & Swapchain
    resize( width, height );

    if ( _window )
    {
      glfwSetWindowUserPointer( _window->getWindow( ), this );

      glfwSetFramebufferSizeCallback( _window->getWindow( ), 
        VulkanApp::resizeCallback );
      //glfwSetWindowRefreshCallback( _window->getWindow( ), VulkanApp::paintCallback );
      glfwSetKeyCallback( _window->getWindow( ), 
        VulkanApp::keyCallback );
      glfwSetMouseButtonCallback( _window->getWindow( ), 
        VulkanApp::mouseButtonCallback );
      glfwSetCursorPosCallback( _window->getWindow( ), 
        VulkanApp::cursorPosCallback );
    }
  }

  VulkanApp::~VulkanApp( void )
  {
    _device->waitIdle( );
    _defaultFramebuffer.reset( );
    std::cout << "DESTROY" << std::endl;
  }

  void VulkanApp::initCapabilities( void )
  {
    const vk::PhysicalDeviceProperties& deviceProps =
      _physicalDevice->getDeviceProperties( );
    const vk::PhysicalDeviceFeatures& deviceFeatures =
      _physicalDevice->getDeviceFeatures( );
    const vk::PhysicalDeviceLimits& deviceLimits = deviceProps.limits;

    DriverVersion driverVersion;
    driverVersion.major = ( ( uint32_t ) ( deviceProps.apiVersion ) >> 22 );
    driverVersion.minor = ( ( uint32_t ) ( deviceProps.apiVersion ) >> 12 ) & 0x3ff;
    driverVersion.release = ( uint32_t ) ( deviceProps.apiVersion ) & 0xfff;
    driverVersion.build = 0;

    caps.setDriverVersion( driverVersion );
    caps.setDeviceName( deviceProps.deviceName );

    // Determine vendor
    switch ( deviceProps.vendorID )
    {
    case 0x10DE:
      std::cout << "GPU NVIDIA" << std::endl;
      caps.setVendor( GPU_NVIDIA );
      break;
    case 0x1002:
      std::cout << "GPU AMD" << std::endl;
      caps.setVendor( GPU_AMD );
      break;
    case 0x163C:
    case 0x8086:
      std::cout << "GPU INTEL" << std::endl;
      caps.setVendor( GPU_INTEL );
      break;
    default:
      std::cout << "GPU UNKNOWN" << std::endl;
      caps.setVendor( GPU_UNKNOWN );
      break;
    };

    if ( deviceFeatures.textureCompressionBC )
      caps.setCapability( RSC_TEXTURE_COMPRESSION_BC );

    if ( deviceFeatures.textureCompressionETC2 )
      caps.setCapability( RSC_TEXTURE_COMPRESSION_ETC2 );

    if ( deviceFeatures.textureCompressionASTC_LDR )
      caps.setCapability( RSC_TEXTURE_COMPRESSION_ASTC );

    caps.setMaxBoundVertexBuffers( deviceLimits.maxVertexInputBindings );
    caps.setNumMultiRenderTargets( deviceLimits.maxColorAttachments );

    caps.setCapability( RSC_COMPUTE_PROGRAM );

    caps.setNumTextureUnits( GpuProgramType::FRAGMENT_PROGRAM,
      deviceLimits.maxPerStageDescriptorSampledImages );
    caps.setNumTextureUnits( GpuProgramType::VERTEX_PROGRAM,
      deviceLimits.maxPerStageDescriptorSampledImages );
    caps.setNumTextureUnits( GpuProgramType::COMPUTE_PROGRAM,
      deviceLimits.maxPerStageDescriptorSampledImages );

    caps.setNumGpuParamBlockBuffers( GpuProgramType::FRAGMENT_PROGRAM,
      deviceLimits.maxPerStageDescriptorUniformBuffers );
    caps.setNumGpuParamBlockBuffers( GpuProgramType::VERTEX_PROGRAM,
      deviceLimits.maxPerStageDescriptorUniformBuffers );
    caps.setNumGpuParamBlockBuffers( GpuProgramType::COMPUTE_PROGRAM,
      deviceLimits.maxPerStageDescriptorUniformBuffers );

    caps.setNumLoadStoreTextureUnits( GpuProgramType::FRAGMENT_PROGRAM,
      deviceLimits.maxPerStageDescriptorStorageImages );
    caps.setNumLoadStoreTextureUnits( GpuProgramType::COMPUTE_PROGRAM,
      deviceLimits.maxPerStageDescriptorStorageImages );

    if ( deviceFeatures.geometryShader )
    {
      caps.setCapability( RSC_GEOMETRY_PROGRAM );
      caps.addShaderProfile( "gs_5_0" );
      caps.setNumTextureUnits( GpuProgramType::GEOMETRY_PROGRAM,
        deviceLimits.maxPerStageDescriptorSampledImages );
      caps.setNumGpuParamBlockBuffers( GpuProgramType::GEOMETRY_PROGRAM,
        deviceLimits.maxPerStageDescriptorUniformBuffers );
      caps.setGeometryProgramNumOutputVertices(
        deviceLimits.maxGeometryOutputVertices );
    }

    if ( deviceFeatures.tessellationShader )
    {
      caps.setCapability( RSC_TESSELLATION_PROGRAM );

      caps.setNumTextureUnits( GpuProgramType::TESS_EVAL_PROGRAM,
        deviceLimits.maxPerStageDescriptorSampledImages );
      caps.setNumTextureUnits( GpuProgramType::TESS_CTRL_PROGRAM,
        deviceLimits.maxPerStageDescriptorSampledImages );

      caps.setNumGpuParamBlockBuffers( GpuProgramType::TESS_EVAL_PROGRAM,
        deviceLimits.maxPerStageDescriptorUniformBuffers );
      caps.setNumGpuParamBlockBuffers( GpuProgramType::TESS_CTRL_PROGRAM,
        deviceLimits.maxPerStageDescriptorUniformBuffers );
    }

    caps.setNumCombinedTextureUnits(
      caps.getNumTextureUnits( GpuProgramType::FRAGMENT_PROGRAM )
      + caps.getNumTextureUnits( GpuProgramType::VERTEX_PROGRAM ) +
      caps.getNumTextureUnits( GpuProgramType::GEOMETRY_PROGRAM )
      + caps.getNumTextureUnits( GpuProgramType::TESS_EVAL_PROGRAM ) +
      caps.getNumTextureUnits( GpuProgramType::TESS_CTRL_PROGRAM )
      + caps.getNumTextureUnits( GpuProgramType::COMPUTE_PROGRAM ) );

    caps.setNumCombinedGpuParamBlockBuffers(
      caps.getNumGpuParamBlockBuffers( GpuProgramType::FRAGMENT_PROGRAM )
      + caps.getNumGpuParamBlockBuffers( GpuProgramType::VERTEX_PROGRAM ) +
      caps.getNumGpuParamBlockBuffers( GpuProgramType::GEOMETRY_PROGRAM )
      + caps.getNumGpuParamBlockBuffers( GpuProgramType::TESS_EVAL_PROGRAM ) +
      caps.getNumGpuParamBlockBuffers( GpuProgramType::TESS_CTRL_PROGRAM )
      + caps.getNumGpuParamBlockBuffers( GpuProgramType::COMPUTE_PROGRAM ) );

    caps.setNumCombinedLoadStoreTextureUnits(
      caps.getNumLoadStoreTextureUnits( GpuProgramType::FRAGMENT_PROGRAM )
      + caps.getNumLoadStoreTextureUnits( GpuProgramType::COMPUTE_PROGRAM ) );

    caps.addShaderProfile( "glsl" );
  }

  bool VulkanApp::checkValidationLayerSupport(
    const std::vector<const char*>& validationLayers )
  {
    std::vector<vk::LayerProperties> availableLayers =
      vk::enumerateInstanceLayerProperties( );

    for ( const char* layerName : validationLayers )
    {
      bool layerFound = false;

      for ( const auto& layerProperties : availableLayers )
      {
        if ( strcmp( layerName, layerProperties.layerName ) == 0 )
        {
          layerFound = true;
          break;
        }
      }

      if ( !layerFound )
      {
        return false;
      }
    }

    return true;
  }

  void VulkanApp::resize( uint32_t width, uint32_t height )
  {
    assert( ( 0 <= width ) && ( 0 <= height ) );

    _defaultFramebuffer.reset( );    // need to be reset, before creating a new one!!
    _defaultFramebuffer.reset( new DefaultFramebuffer( _device, _surface,
      _colorFormat, _colorSpace, _depthFormat, _renderPass ) );
    
    // todo: WHY FAIL IN UBUNTU?? assert( _defaultFramebuffer->getExtent( ) == vk::Extent2D( width, height ) );

    doResize( width, height );
  }

  void VulkanApp::doResize( uint32_t width, uint32_t height )
  {
  }

  void VulkanApp::paint( void )
  {
    // Get the index of the next available swapchain image:
    _defaultFramebuffer->acquireNextFrame( );
    doPaint( );
    _defaultFramebuffer->present( _graphicsQueue, _renderComplete );

    _device->waitIdle( ); // TODO: Neccesary ??
  }

  void VulkanApp::doPaint( void )
  {
    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    float timeValue = glfwGetTime( );
    float greenValue = sin( timeValue ) / 2.0f + 0.5f;

    std::cout << greenValue << std::endl;

    std::array<float, 4> ccv = { 0.0f, greenValue, 0.0f, 1.0f };
    commandBuffer->begin( );
    commandBuffer->beginRenderPass( _renderPass, _defaultFramebuffer->getFramebuffer( ),
      vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ), { vk::ClearValue( ccv ),
      vk::ClearValue( vk::ClearDepthStencilValue( 1.0f, 0 ) ) },
      vk::SubpassContents::eInline );
    commandBuffer->setViewport( 0, { vk::Viewport( 0.0f, 0.0f,
      ( float ) _defaultFramebuffer->getExtent( ).width,
      ( float ) _defaultFramebuffer->getExtent( ).height, 0.0f, 1.0f ) } );
    commandBuffer->setScissor( 0, { vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ) } );
    commandBuffer->endRenderPass( );
    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }

  void VulkanApp::paintCallback( GLFWwindow * window )
  {
    reinterpret_cast< VulkanApp* >(
      glfwGetWindowUserPointer( window ) )->paint( );
  }

  void VulkanApp::resizeCallback( GLFWwindow *window, int width, int height )
  {
    reinterpret_cast< VulkanApp* >(
      glfwGetWindowUserPointer( window ) )->resize( width, height );
  }

  void VulkanApp::cursorPosCallback( GLFWwindow * window, double xPos, double yPos )
  {
    reinterpret_cast< VulkanApp* >(
      glfwGetWindowUserPointer( window ) )->cursorPosEvent( xPos, yPos );
  }

  void VulkanApp::keyCallback( GLFWwindow* window, int key, int scancode,
    int action, int mods )
  {
    reinterpret_cast< VulkanApp* >(
      glfwGetWindowUserPointer( window ) )->keyEvent( key, scancode, action, mods );
  }

  void VulkanApp::mouseButtonCallback( GLFWwindow * window, int button,
    int action, int mods )
  {
    reinterpret_cast< VulkanApp* >(
      glfwGetWindowUserPointer( window ) )->mouseButtonEvent( button, action, mods );
  }

  void VulkanApp::cursorPosEvent( double, double )
  {
  }

  void VulkanApp::keyEvent( int, int, int, int )
  {
  }

  void VulkanApp::mouseButtonEvent( int, int, int )
  {
  }
}