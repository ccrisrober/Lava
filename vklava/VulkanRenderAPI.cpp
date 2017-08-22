#include "VulkanRenderAPI.h"

#include <assert.h>

#include <functional>
#include <fstream>

#include <sstream>
#include "routes.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VulkanPipelineState.h"
#include "VulkanSampler.h"
#include "VulkanFramebuffer.h"

namespace lava
{
  VkResult CreateDebugReportCallbackEXT( VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback )
  {
    auto func = ( PFN_vkCreateDebugReportCallbackEXT ) vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT" );
    if ( func != nullptr )
    {
      return func( instance, pCreateInfo, pAllocator, pCallback );
    }
    else
    {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  void DestroyDebugReportCallbackEXT( VkInstance instance,
    VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator )
  {
    auto func = ( PFN_vkDestroyDebugReportCallbackEXT ) vkGetInstanceProcAddr(
      instance, "vkDestroyDebugReportCallbackEXT" );
    if ( func != nullptr )
    {
      func( instance, callback, pAllocator );
    }
  }
  
  VkBool32 debugMsgCallback( VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
    size_t location, int32_t msgCode, const char* pLayerPrefix,
    const char* pMsg, void* pUserData )
  {
    std::stringstream message;

    // Determine prefix
    if ( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
      message << "ERROR";

    if ( flags & VK_DEBUG_REPORT_WARNING_BIT_EXT )
      message << "WARNING";

    if ( flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT )
      message << "PERFORMANCE";

    if ( flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT )
      message << "INFO";

    if ( flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT )
      message << "DEBUG";

    message << ": [" << pLayerPrefix << "] Code " << msgCode << ": "
      << pMsg << std::endl;

    if ( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
      std::cerr << message.str( ) << std::endl;
    else if ( flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags &
      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT )
      std::cerr << message.str( ) << std::endl;
    else
      std::cerr << message.str( ) << std::endl;

    // Don't abort calls that caused a validation message
    return VK_FALSE;
  }

  VulkanRenderAPI::VulkanRenderAPI( )
    : _instance( nullptr )
#ifndef NDEBUG
    , _debugCallback( VK_NULL_HANDLE )
#endif
  {
    glfwInit( );

    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    //glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    _window = glfwCreateWindow( WIDTH, HEIGHT, "Vulkan", nullptr, nullptr );

    glfwSetWindowUserPointer( _window, this );
  }

  VulkanRenderAPI::~VulkanRenderAPI( void )
  {
  }

  void VulkanRenderAPI::cleanupSwapChain( void )
  {
    VkDevice logicalDevice = _getPresentDevice( )->getLogical( );

    vkDestroyImageView( logicalDevice, depthImageView, nullptr );
    vkDestroyImage( logicalDevice, depthImage, nullptr );
    vkFreeMemory( logicalDevice, depthImageMemory, nullptr );

    for ( size_t i = 0; i < swapChainFramebuffers.size( ); ++i )
    {
      vkDestroyFramebuffer( logicalDevice, swapChainFramebuffers[ i ], nullptr );
    }

    vkFreeCommandBuffers( logicalDevice, commandPool, 
      static_cast<uint32_t>( commandBuffers.size( ) ), commandBuffers.data( ) );

    vkDestroyPipeline( logicalDevice, graphicsPipeline, nullptr );
    vkDestroyPipelineLayout( logicalDevice, pipelineLayout, nullptr );
    vkDestroyRenderPass( logicalDevice, renderPass, nullptr );
  }
  
  void VulkanRenderAPI::cleanup( void )
  {
    cleanupSwapChain( );
    VkDevice logicalDevice = _getPresentDevice( )->getLogical( );
    _getPresentDevice( )->waitIdle( );

    delete textureSampler; //vkDestroySampler( logicalDevice, textureSampler, nullptr );
    vkDestroyImageView( logicalDevice, textureImageView, nullptr );

    vkDestroyImage( logicalDevice, textureImage, nullptr );
    _getPresentDevice( )->freeMemory( textureImageMemory );

    vkDestroyDescriptorPool( logicalDevice, descriptorPool, nullptr );
    vkDestroyDescriptorSetLayout( logicalDevice, descriptorSetLayout, nullptr );

    vkDestroyBuffer( logicalDevice, uniformBufferVS.buffer, nullptr );
    _getPresentDevice( )->freeMemory( uniformBufferVS.memory );

    vkDestroyBuffer( logicalDevice, verticesBuffer.buffer, nullptr );
    _getPresentDevice( )->freeMemory( verticesBuffer.memory );

    vkDestroyBuffer( logicalDevice, indicesBuffer.buffer, nullptr );
    _getPresentDevice( )->freeMemory( indicesBuffer.memory );

    delete renderFinishedSemaphore;
    delete imageAvailableSemaphore;

    vkDestroyCommandPool( logicalDevice, commandPool, nullptr );

#ifndef NDEBUG
    if ( _debugCallback != 0 )
    {
      DestroyDebugReportCallbackEXT( _instance, _debugCallback, nullptr );
    }
#endif

    //_swapChain.reset( );

    _renderWindow.reset( ); //vkDestroySurfaceKHR( _instance, _surface, nullptr );

    _primaryDevices.clear( );
    _devices.clear( );
    vkDestroyInstance( _instance, nullptr );

    glfwDestroyWindow( _window );

    glfwTerminate( );
  }

  void VulkanRenderAPI::initialize( void )
  {
    // Create instance
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "App Name";
    appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.pEngineName = "FooEngine";
    appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> exts;

    uint32_t extensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions( &extensionCount );

    extensionCount = 0;
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
    std::vector<VkExtensionProperties> extensions_( extensionCount );
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions_.data( ) );
    std::cout << "available extensions:" << std::endl;

    for ( const auto& extension : extensions_ )
    {
      std::cout << "\t" << extension.extensionName << std::endl;
    }

#ifndef NDEBUG
    std::vector<const char*> layers =
    {
      "VK_LAYER_LUNARG_standard_validation"
    };
    checkValidationLayerSupport( layers );
    std::vector<const char*> extensions =
    {
      glfwExtensions[ 0 ],	// Surface extension
      glfwExtensions[ 1 ],	// OS specific surface extension
      "VK_EXT_debug_report"
    };
#else
    std::vector<const char*> layers;
    std::vector<const char*> extensions =
    {
      glfwExtensions[ 0 ],	// Surface extension
      glfwExtensions[ 1 ],	// OS specific surface extension
    };
#endif

    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = static_cast< uint32_t >( layers.size( ) );
    instanceInfo.ppEnabledLayerNames = layers.data( );
    instanceInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.size( ) );
    instanceInfo.ppEnabledExtensionNames = extensions.data( );

    VkResult res = vkCreateInstance( &instanceInfo, nullptr, &_instance );
    assert( res == VK_SUCCESS );

#ifndef NDEBUG
    // Set debug callback
    VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
      VK_DEBUG_REPORT_WARNING_BIT_EXT |
      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

    VkDebugReportCallbackCreateInfoEXT debugInfo;
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debugInfo.pNext = nullptr;
    debugInfo.pfnCallback = ( PFN_vkDebugReportCallbackEXT ) debugMsgCallback;
    debugInfo.flags = debugFlags;

    if ( CreateDebugReportCallbackEXT( _instance, &debugInfo, nullptr,
      &_debugCallback ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to set up debug callback!" );
    }
#endif

    uint32_t _numDevices = 0;

    // Enumerate all devices
    res = vkEnumeratePhysicalDevices( _instance, &_numDevices, nullptr );
    assert( res == VK_SUCCESS );

    if ( _numDevices == 0 )
    {
      throw std::runtime_error( "failed to find GPUs with Vulkan support!" );
    }

    std::vector<VkPhysicalDevice> physicalDevices( _numDevices );
    res = vkEnumeratePhysicalDevices( _instance, &_numDevices, physicalDevices.data( ) );

    _devices.resize( _numDevices );
    for ( uint32_t i = 0; i < _numDevices; ++i )
    {
      _devices[ i ] = std::make_shared<VulkanDevice>( physicalDevices[ i ], i );
    }

    // Find primary device
    // Note: MULTIGPU - Detect multiple similar devices here if supporting multi-GPU
    for ( uint32_t i = 0; i < _numDevices; ++i )
    {
      bool isPrimary = _devices[ i ]->getDeviceProperties( ).deviceType
        == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

      if ( isPrimary )
      {
        _devices[ i ]->setIsPrimary( );
        _primaryDevices.push_back( _devices[ i ] );
        break;
      }
    }

    if ( _primaryDevices.empty( ) )
    {
      _primaryDevices.push_back( _devices.front( ) );
    }

    _renderWindow = std::make_shared<RenderWindow>( *this );



    // INIT CAPABILITES
    std::shared_ptr<VulkanDevice> presentDevice = _getPresentDevice( );
    VkPhysicalDevice physicalDevice = presentDevice->getPhysical( );

    const VkPhysicalDeviceProperties& deviceProps = 
      presentDevice->getDeviceProperties( );
    const VkPhysicalDeviceFeatures& deviceFeatures = 
      presentDevice->getDeviceFeatures( );
    const VkPhysicalDeviceLimits& deviceLimits = deviceProps.limits;

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

    caps.setNumTextureUnits( GPT_FRAGMENT_PROGRAM, 
      deviceLimits.maxPerStageDescriptorSampledImages );
    caps.setNumTextureUnits( GPT_VERTEX_PROGRAM, 
      deviceLimits.maxPerStageDescriptorSampledImages );
    caps.setNumTextureUnits( GPT_COMPUTE_PROGRAM, 
      deviceLimits.maxPerStageDescriptorSampledImages );

    caps.setNumGpuParamBlockBuffers( GPT_FRAGMENT_PROGRAM, 
      deviceLimits.maxPerStageDescriptorUniformBuffers );
    caps.setNumGpuParamBlockBuffers( GPT_VERTEX_PROGRAM, 
      deviceLimits.maxPerStageDescriptorUniformBuffers );
    caps.setNumGpuParamBlockBuffers( GPT_COMPUTE_PROGRAM, 
      deviceLimits.maxPerStageDescriptorUniformBuffers );

    caps.setNumLoadStoreTextureUnits( GPT_FRAGMENT_PROGRAM, 
      deviceLimits.maxPerStageDescriptorStorageImages );
    caps.setNumLoadStoreTextureUnits( GPT_COMPUTE_PROGRAM, 
      deviceLimits.maxPerStageDescriptorStorageImages );

    if ( deviceFeatures.geometryShader )
    {
      caps.setCapability( RSC_GEOMETRY_PROGRAM );
      caps.addShaderProfile( "gs_5_0" );
      caps.setNumTextureUnits( GPT_GEOMETRY_PROGRAM, 
        deviceLimits.maxPerStageDescriptorSampledImages );
      caps.setNumGpuParamBlockBuffers( GPT_GEOMETRY_PROGRAM, 
        deviceLimits.maxPerStageDescriptorUniformBuffers );
      caps.setGeometryProgramNumOutputVertices( 
        deviceLimits.maxGeometryOutputVertices );
    }

    if ( deviceFeatures.tessellationShader )
    {
      caps.setCapability( RSC_TESSELLATION_PROGRAM );

      caps.setNumTextureUnits( GPT_TESS_EVAL_PROGRAM, 
        deviceLimits.maxPerStageDescriptorSampledImages );
      caps.setNumTextureUnits( GPT_TESS_CTRL_PROGRAM, 
        deviceLimits.maxPerStageDescriptorSampledImages );

      caps.setNumGpuParamBlockBuffers( GPT_TESS_EVAL_PROGRAM, 
        deviceLimits.maxPerStageDescriptorUniformBuffers );
      caps.setNumGpuParamBlockBuffers( GPT_TESS_CTRL_PROGRAM, 
        deviceLimits.maxPerStageDescriptorUniformBuffers );
    }

    caps.setNumCombinedTextureUnits( 
      caps.getNumTextureUnits( GPT_FRAGMENT_PROGRAM )
      + caps.getNumTextureUnits( GPT_VERTEX_PROGRAM ) + 
      caps.getNumTextureUnits( GPT_GEOMETRY_PROGRAM )
      + caps.getNumTextureUnits( GPT_TESS_EVAL_PROGRAM ) + 
      caps.getNumTextureUnits( GPT_TESS_CTRL_PROGRAM )
      + caps.getNumTextureUnits( GPT_COMPUTE_PROGRAM ) );

    caps.setNumCombinedGpuParamBlockBuffers( 
      caps.getNumGpuParamBlockBuffers( GPT_FRAGMENT_PROGRAM )
      + caps.getNumGpuParamBlockBuffers( GPT_VERTEX_PROGRAM ) + 
      caps.getNumGpuParamBlockBuffers( GPT_GEOMETRY_PROGRAM )
      + caps.getNumGpuParamBlockBuffers( GPT_TESS_EVAL_PROGRAM ) + 
      caps.getNumGpuParamBlockBuffers( GPT_TESS_CTRL_PROGRAM )
      + caps.getNumGpuParamBlockBuffers( GPT_COMPUTE_PROGRAM ) );

    caps.setNumCombinedLoadStoreTextureUnits( 
      caps.getNumLoadStoreTextureUnits( GPT_FRAGMENT_PROGRAM )
      + caps.getNumLoadStoreTextureUnits( GPT_COMPUTE_PROGRAM ) );

    caps.addShaderProfile( "glsl" );

    //__init__( );
  }

  void VulkanRenderAPI::__init__( )
  {
    VulkanGpuProgram * vertShaderModule = new VulkanGpuProgram( GPU_PROGRAM_DESC (
      VKLAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/vert.spv" ),
      GpuProgramType::GPT_VERTEX_PROGRAM
    ));
    VulkanGpuProgram * fragShaderModule = new VulkanGpuProgram( GPU_PROGRAM_DESC (
      VKLAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/frag.spv" ),
      GpuProgramType::GPT_VERTEX_PROGRAM
    ));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule->getShaderModule( )->getHandle( );
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule->getShaderModule( )->getHandle( );
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
      vertShaderStageInfo, fragShaderStageInfo
    };



    // RENDER PASSES
    VkDevice logicalDevice = _getPresentDevice( )->getLogical( );

    VULKAN_FRAMEBUFFER_DESC fboDesc;
    fboDesc.numColorAttachments = 1;
    fboDesc.colorFormats.emplace_back( _renderWindow->_colorFormat );
    fboDesc.hasDepth = true;
    fboDesc.depthFormat = findDepthFormat( );
    VulkanFramebuffer vfb( _getPresentDevice( ), fboDesc );

    renderPass = vfb.renderPass;

    /*// UBO DESCRIPTOR
    VkDescriptorSetLayoutBinding uboLayoutBinding = { };
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // TEXTURE DESCRIPTOR
    VkDescriptorSetLayoutBinding samplerLayoutBinding = { };
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };*/

    std::vector< VkDescriptorSetLayoutBinding > bindings =
    {
      // Binding 0 : Vertex shader uniform buffer
      descriptorSetLayoutBinding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT,
        0 ),

      // Binding 1 : Fragment shader image sampler
      descriptorSetLayoutBinding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        1 )
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = { };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>( bindings.size( ) );
    layoutInfo.pBindings = bindings.data( );

    if ( vkCreateDescriptorSetLayout( logicalDevice, &layoutInfo, 
      nullptr, &descriptorSetLayout ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create descriptor set layout!" );
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if ( vkCreatePipelineLayout( logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create pipeline layout!" );
    }

    // FIXED FUNCTIONS
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { };
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


    auto bindingDescription = Vertex::getBindingDescription( );
    auto attributeDescriptions = Vertex::getAttributeDescriptions( );

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast< uint32_t >( attributeDescriptions.size( ) );
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data( );


    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { };
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = 1; // Spec says this need to be at least 1...
    viewportState.scissorCount = 1;
    viewportState.pViewports = nullptr; // Dynamic
    viewportState.pScissors = nullptr; // Dynamic

    VkPipelineRasterizationStateCreateInfo rasterizer = { };
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE; // When enabled, perform shading per sample instead of per pixel (more expensive, essentially FSAA)
    multisampling.minSampleShading = 1.0f; // Minimum percent of samples to run full shading for when sampleShadingEnable is enabled (1.0f to run for all)
    multisampling.pSampleMask = nullptr; // Normally one bit for each sample: e.g. 0x0000000F to enable all samples in a 4-sample setup
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil = { };
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { };
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = { };
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[ 0 ] = 0.0f;
    colorBlending.blendConstants[ 1 ] = 0.0f;
    colorBlending.blendConstants[ 2 ] = 0.0f;
    colorBlending.blendConstants[ 3 ] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo = { };
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    _dynamicStates[ 0 ] = VK_DYNAMIC_STATE_VIEWPORT;
    _dynamicStates[ 1 ] = VK_DYNAMIC_STATE_SCISSOR;
    _dynamicStates[ 2 ] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;

    uint32_t numDynamicStates = sizeof( _dynamicStates ) / sizeof( _dynamicStates[ 0 ] );
    assert( numDynamicStates == 3 );

    _dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    _dynamicStateInfo.pNext = nullptr;
    _dynamicStateInfo.flags = 0;
    _dynamicStateInfo.dynamicStateCount = numDynamicStates;
    _dynamicStateInfo.pDynamicStates = _dynamicStates;


    pipelineInfo.pDynamicState = &_dynamicStateInfo;

    if ( vkCreateGraphicsPipelines( logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create graphics pipeline!" );
    }


    delete fragShaderModule;
    delete vertShaderModule;


    // COMMAND POOL
    VkCommandPoolCreateInfo poolCI;
    poolCI.pNext = nullptr;
    poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.queueFamilyIndex = _getPresentDevice( )->getQueueFamily(
      GpuQueueType::GPUT_GRAPHICS );
    if ( vkCreateCommandPool( logicalDevice, &poolCI, nullptr,
      &commandPool ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create command pool!" );
    }



    {
      VkFormat depthFormat = findDepthFormat( );
      // Depth texture
      createImage( _renderWindow->_swapChain->getWidth( ),
        _renderWindow->_swapChain->getHeight( ), depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory );
      depthImageView = createImageView( depthImage, depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT );

      transitionImageLayout( depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
    }




    // FRAMEBUFFERS
    swapChainFramebuffers.resize( _renderWindow->_swapChain->swapChainImageViews.size( ) );

    for ( size_t i = 0; i < _renderWindow->_swapChain->swapChainImageViews.size( ); i++ )
    {
      std::array<VkImageView, 2> attachments = {
        _renderWindow->_swapChain->swapChainImageViews[ i ],
        depthImageView
      };

      VkFramebufferCreateInfo framebufferInfo = { };
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>( attachments.size( ) );
      framebufferInfo.pAttachments = attachments.data( );
      framebufferInfo.width = _renderWindow->_swapChain->getWidth( );
      framebufferInfo.height = _renderWindow->_swapChain->getHeight( );
      framebufferInfo.layers = 1;

      if ( vkCreateFramebuffer( logicalDevice,
        &framebufferInfo, nullptr, &swapChainFramebuffers[ i ] ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create framebuffer!" );
      }
    }

    auto createBuffer = [ &]( VkDeviceSize size, VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties, VkBuffer& buffer,
      VkDeviceMemory& bufferMemory )
    {
      VkBufferCreateInfo bufferInfo = { };
      bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferInfo.size = size;
      bufferInfo.usage = usage;
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if ( vkCreateBuffer( logicalDevice, &bufferInfo, nullptr, &buffer ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create buffer!" );
      }

      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements( logicalDevice, buffer, &memRequirements );

      bufferMemory = _getPresentDevice( )->allocateMemReqMemory(
        memRequirements,
        properties );

      vkBindBufferMemory( logicalDevice, buffer, bufferMemory, 0 );
    };

    auto copyBufferToImage = [ & ]( VkBuffer buffer, VkImage image, 
      uint32_t width, uint32_t height )
    {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands( );

      VkBufferImageCopy region = { };
      region.bufferOffset = 0;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;

      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = 0;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = 1;

      region.imageOffset = { 0, 0, 0 };
      region.imageExtent = {
        width,
        height,
        1
      };

      vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
      );

      endSingleTimeCommands( commandBuffer );
    };

    {
      // TEXTURE IMAGE CREATION
      int texWidth, texHeight, texChannels;
      stbi_uc* pixels = stbi_load( ( VKLAVA_EXAMPLES_RESOURCES_ROUTE + 
        std::string( "/uv_checker.png" ) ).c_str( ),
        &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
      if ( pixels == nullptr )
      {
        std::cerr << stbi_failure_reason( ) << std::endl;
        throw new std::exception( stbi_failure_reason( ) );
      }
      VkDeviceSize imageSize = texWidth * texHeight * 4;

      if ( !pixels )
      {
        throw std::runtime_error( "failed to load texture image!" );
      }

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      createBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory );

      void* data;
      vkMapMemory( logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data );
      memcpy( data, pixels, static_cast<size_t>( imageSize ) );
      vkUnmapMemory( logicalDevice, stagingBufferMemory );

      stbi_image_free( pixels );

      createImage( texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory );

      transitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_UNORM, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
      copyBufferToImage( stagingBuffer, textureImage, 
        static_cast<uint32_t>( texWidth ), static_cast<uint32_t>( texHeight ) );
      transitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_UNORM, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

      vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
      _getPresentDevice( )->freeMemory( stagingBufferMemory );


      textureImageView = createImageView( textureImage, 
        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

      SAMPLER_STATE_DESC samplerDesc;
      samplerDesc.mipFilter = FO_ANISOTROPIC;
      samplerDesc.maxAniso = 1;

      textureSampler = new VulkanSamplerState( samplerDesc );
    }

    auto copyBuffer = [ &] ( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size )
    {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands( );

      VkBufferCopy copyRegion = { };
      copyRegion.size = size;
      vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

      endSingleTimeCommands( commandBuffer );
    };

    // Vertex buffer creation
    {
      VkDeviceSize bufferSize = sizeof( vertices[ 0 ] ) * vertices.size( );

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      
      createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory );

      void* data;
      vkMapMemory( logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data );
      memcpy( data, vertices.data( ), ( size_t ) bufferSize );
      vkUnmapMemory( logicalDevice, stagingBufferMemory );

      createBuffer( bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, verticesBuffer.buffer, verticesBuffer.memory );

      copyBuffer( stagingBuffer, verticesBuffer.buffer, bufferSize );

      vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
      vkFreeMemory( logicalDevice, stagingBufferMemory, nullptr );
    }

    // Index buffer creation
    {
      VkDeviceSize bufferSize = sizeof( indices[ 0 ] ) * indices.size( );

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory );

      void* data;
      vkMapMemory( logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data );
      memcpy( data, indices.data( ), ( size_t ) bufferSize );
      vkUnmapMemory( logicalDevice, stagingBufferMemory );

      createBuffer( bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indicesBuffer.buffer, 
        indicesBuffer.memory );

      copyBuffer( stagingBuffer, indicesBuffer.buffer, bufferSize );

      vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
      vkFreeMemory( logicalDevice, stagingBufferMemory, nullptr );

      indicesBuffer.count = indices.size( );
    }

    // UBO buffer creation
    {
      VkDeviceSize bufferSize = sizeof( UniformBufferObject );
      createBuffer( bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBufferVS.buffer, uniformBufferVS.memory );
    }
    {
      // createDescriptorPool
      std::array<VkDescriptorPoolSize, 2> poolSizes = { };
      poolSizes[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSizes[ 0 ].descriptorCount = 1;
      poolSizes[ 1 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      poolSizes[ 1 ].descriptorCount = 1;

      VkDescriptorPoolCreateInfo poolInfo = { };
      poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size( ) );
      poolInfo.pPoolSizes = poolSizes.data( );
      poolInfo.maxSets = 1;

      if ( vkCreateDescriptorPool( logicalDevice, &poolInfo, nullptr, &descriptorPool )
        != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create descriptor pool!" );
      }
    }

    {
      // createDescriptorSet
      VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
      VkDescriptorSetAllocateInfo allocInfo = { };
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = descriptorPool;
      allocInfo.descriptorSetCount = 1;
      allocInfo.pSetLayouts = layouts;

      if ( vkAllocateDescriptorSets( logicalDevice, &allocInfo, &descriptorSet )
        != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to allocate descriptor set!" );
      }

      uniformBufferVS.descriptor = { };

      uniformBufferVS.descriptor.buffer = uniformBufferVS.buffer;
      uniformBufferVS.descriptor.offset = 0;
      uniformBufferVS.descriptor.range = sizeof( UniformBufferObject );

      VkDescriptorImageInfo imageInfo = { };
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = textureImageView;
      imageInfo.sampler = textureSampler->getResource( )->getHandle( );

      std::array<VkWriteDescriptorSet, 2> descriptorWrites = { };

      descriptorWrites[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[ 0 ].dstSet = descriptorSet;
      descriptorWrites[ 0 ].dstBinding = 0;
      descriptorWrites[ 0 ].dstArrayElement = 0;
      descriptorWrites[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[ 0 ].descriptorCount = 1;
      descriptorWrites[ 0 ].pBufferInfo = &uniformBufferVS.descriptor;

      descriptorWrites[ 1 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[ 1 ].dstSet = descriptorSet;
      descriptorWrites[ 1 ].dstBinding = 1;
      descriptorWrites[ 1 ].dstArrayElement = 0;
      descriptorWrites[ 1 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[ 1 ].descriptorCount = 1;
      descriptorWrites[ 1 ].pImageInfo = &imageInfo;

      vkUpdateDescriptorSets( logicalDevice, 
        static_cast<uint32_t>( descriptorWrites.size( ) ), 
        descriptorWrites.data( ), 0, nullptr );
    }


    // COMMAND BUFFERS
    commandBuffers.resize( swapChainFramebuffers.size( ) );
    VkCommandBufferAllocateInfo cmdBufAllocInfo = { };
    cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocInfo.commandPool = commandPool;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocInfo.commandBufferCount = ( uint32_t ) commandBuffers.size( );

    if ( vkAllocateCommandBuffers( logicalDevice, &cmdBufAllocInfo,
      commandBuffers.data( ) ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to allocate command buffers!" );
    }
    // Starting command buffer recording
    uint32_t i = 0;
    for ( const VkCommandBuffer& cmd: commandBuffers )
    {
      VkCommandBufferBeginInfo beginInfo = { };
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

      vkBeginCommandBuffer( cmd, &beginInfo );

      // Starting a render pass
      VkRenderPassBeginInfo renderPassBeginInfo;
      renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassBeginInfo.pNext = nullptr;

      renderPassBeginInfo.renderPass = renderPass;
      renderPassBeginInfo.framebuffer = swapChainFramebuffers[ i ];
      renderPassBeginInfo.renderArea.offset = { 0, 0 };
      renderPassBeginInfo.renderArea.extent =
      {
        _renderWindow->_swapChain->getWidth( ),
        _renderWindow->_swapChain->getHeight( )
      };

      std::array<VkClearValue, 2> clearValues = { };
      clearValues[ 0 ].color = { 0.0f, 0.0f, 0.0f, 1.0f };
      clearValues[ 1 ].depthStencil = { 1.0f, 0 };

      renderPassBeginInfo.clearValueCount = static_cast<uint32_t>( clearValues.size( ) );
      renderPassBeginInfo.pClearValues = clearValues.data( );

      vkCmdBeginRenderPass( cmd, &renderPassBeginInfo,
        VK_SUBPASS_CONTENTS_INLINE );

        // Basic drawing commands
        vkCmdBindPipeline( commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );

          VkViewport viewport;
          viewport.x = 0.0f;
          viewport.y = 0.0f;
          viewport.width = ( float ) _renderWindow->_swapChain->getWidth( );
          viewport.height = ( float ) _renderWindow->_swapChain->getHeight( );
          viewport.minDepth = 0.0f;
          viewport.maxDepth = 1.0f;

          vkCmdSetViewport( commandBuffers[ i ], 0, 1, &viewport );

          VkRect2D scissorRect;
          scissorRect.offset = { 0, 0 };
          scissorRect.extent =
          {
            _renderWindow->_swapChain->getWidth( ),
            _renderWindow->_swapChain->getHeight( )
          };

          vkCmdSetScissor( commandBuffers[ i ], 0, 1, &scissorRect );

        vkCmdBindPipeline( commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );

        VkBuffer vertexBuffers[ ] = { verticesBuffer.buffer };
        VkDeviceSize offsets[ ] = { 0 };
        vkCmdBindVertexBuffers( commandBuffers[ i ], 0, 1, vertexBuffers, offsets );

        vkCmdBindDescriptorSets( commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, 
          pipelineLayout, 0, 1, &descriptorSet, 0, nullptr );

        vkCmdBindIndexBuffer( commandBuffers[ i ], indicesBuffer.buffer, 0,
          VK_INDEX_TYPE_UINT16 );

        vkCmdDrawIndexed( commandBuffers[ i ], indicesBuffer.count, 1, 0, 0, 0 );

      vkCmdEndRenderPass( commandBuffers[ i ] );

      if ( vkEndCommandBuffer( commandBuffers[ i ] ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to record command buffer!" );
      }
      ++i;
    }

    // SEMAPHORES
    imageAvailableSemaphore = new VulkanSemaphore( _getPresentDevice( ) );
    renderFinishedSemaphore = new VulkanSemaphore( _getPresentDevice( ) );
  }

  bool VulkanRenderAPI::checkValidationLayerSupport( 
    const std::vector<const char*>& validationLayers )
  {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

    std::vector<VkLayerProperties> availableLayers( layerCount );
    vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data( ) );

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
        //std::cout << layerProperties.layerName << std::endl;
      }

      if ( !layerFound )
      {
        return false;
      }
    }

    return true;
  }
}