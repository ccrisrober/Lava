#include "VulkanRenderAPI.h"

#include <assert.h>

#include <functional>
#include <fstream>

#include <sstream>
#include "routes.h"

#include <iomanip>

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

    vkDestroyImageView( logicalDevice, depthStencilTex.view, nullptr );
    vkDestroyImage( logicalDevice, depthStencilTex.image, nullptr );
    vkFreeMemory( logicalDevice, depthStencilTex.mem, nullptr );

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

    delete semaphores.presentComplete;
    delete semaphores.renderComplete;

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
	_instance.destroy();

    glfwDestroyWindow( _window );

    glfwTerminate( );
  }

  void VulkanRenderAPI::initialize( void )
  {
    // Create instance
	vk::ApplicationInfo appInfo (
		"App Name",
		VK_MAKE_VERSION(1, 0, 0),
		"FooEngine",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_0
	);

    std::vector<const char*> exts;

    uint32_t extensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions( &extensionCount );

    extensionCount = 0;
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
    std::vector<VkExtensionProperties> extensions_( extensionCount );
    vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions_.data( ) );

#ifndef NDEBUG
    std::vector<const char*> layers =
    {
      "VK_LAYER_LUNARG_standard_validation",
      //"VK_LAYER_LUNARG_api_dump"

    };
    checkValidationLayerSupport( layers );
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
      std::cout << "[Vulkan init] extensions: name=" << extension.extensionName <<
        ", enabled=";

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
		  vk::enumerateInstanceLayerProperties();

      std::cout << "Instance layers: " << layersPropList.size( ) << " item(s)\n";
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
        std::cout << " " << std::left << std::setw( 40 ) << l.layerName << " (";
        if ( founded ) std::cout << "1";
        else std::cout << "0";
        std::cout << ")| " << std::setw( 100 ) << l.description << std::endl;
      }
    }

	_instance = vk::createInstance(
		vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			layers.size(),
			layers.data(),
			extensions.size(),
			extensions.data()
		)
	);

#ifndef NDEBUG
	vk::DebugReportFlagsEXT debugFlags = vk::DebugReportFlagBitsEXT::eError |
		vk::DebugReportFlagBitsEXT::eWarning |
		vk::DebugReportFlagBitsEXT::ePerformanceWarning;

	vk::DebugReportCallbackCreateInfoEXT debugInfo;
	debugInfo.setPfnCallback((PFN_vkDebugReportCallbackEXT)debugMsgCallback);
	debugInfo.setFlags(debugFlags);

	_instance.createDebugReportCallbackEXT(debugInfo);
#endif

    uint32_t _numDevices = 0;

    // Enumerate all devices
    /*res = vkEnumeratePhysicalDevices( _instance, &_numDevices, nullptr );
    assert( res == VK_SUCCESS );

    if ( _numDevices == 0 )
    {
      throw std::runtime_error( "failed to find GPUs with Vulkan support!" );
    }

    std::vector<VkPhysicalDevice> physicalDevices( _numDevices );
    res = vkEnumeratePhysicalDevices( _instance, &_numDevices, physicalDevices.data( ) );*/
	auto physicalDevices = _instance.enumeratePhysicalDevices();



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
        == vk::PhysicalDeviceType::eDiscreteGpu;

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

    initCapabilities( );
  }
  
  void VulkanRenderAPI::initCapabilities( void )
  {
    std::shared_ptr<VulkanDevice> presentDevice = _getPresentDevice( );
    vk::PhysicalDevice physicalDevice = presentDevice->getPhysical( );

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

  void VulkanRenderAPI::__init__( )
  {
    VulkanGpuProgram * vertShaderModule = new VulkanGpuProgram( GPU_PROGRAM_DESC (
      VKLAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/vert.spv" ),
      GpuProgramType::VERTEX_PROGRAM
    ));
    VulkanGpuProgram * fragShaderModule = new VulkanGpuProgram( GPU_PROGRAM_DESC (
      VKLAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/frag.spv" ),
      GpuProgramType::FRAGMENT_PROGRAM
    ));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = vertShaderModule->getShaderModule( )->getHandle( );
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = fragShaderModule->getShaderModule( )->getHandle( );
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
      vertShaderStageInfo, fragShaderStageInfo
    };



    // RENDER PASSES
    vk::Device logicalDevice = _getPresentDevice( )->getLogical( );

    VULKAN_FRAMEBUFFER_DESC fboDesc;
    fboDesc.numColorAttachments = 1;
    fboDesc.colorFormats.emplace_back( _renderWindow->_colorFormat );
    fboDesc.hasDepth = true;
    fboDesc.depthFormat = findDepthFormat( );
    VulkanFramebuffer vfb( _getPresentDevice( ), fboDesc );

    renderPass = vfb.renderPass;

    std::vector< vk::DescriptorSetLayoutBinding > bindings =
    {
      // Binding 0 : Vertex shader uniform buffer
      descriptorSetLayoutBinding(
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex,
        0 ),

      // Binding 1 : Fragment shader image sampler
      descriptorSetLayoutBinding(
		  vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment,
        1 )
    };
    bindings.resize( bindings.size( ) );

    vk::DescriptorSetLayoutCreateInfo layoutInfo = { };
    layoutInfo.bindingCount = static_cast<uint32_t>( bindings.size( ) );
    layoutInfo.pBindings = bindings.data( );

	descriptorSetLayout = logicalDevice.createDescriptorSetLayout(layoutInfo);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    // PUSH CONSTANTS
	vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eFragment;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof( pushConstants );

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    // PUSH CONSTANTS \\

	pipelineLayout = logicalDevice.createPipelineLayout(pipelineLayoutInfo);

    // Create the Buffer resource metadata information
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

    auto bindingDescription = Vertex::getBindingDescription( );
    auto attributeDescriptions = Vertex::getAttributeDescriptions( );

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast< uint32_t >( attributeDescriptions.size( ) );
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data( );


    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


	vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1; // Spec says this need to be at least 1...
    viewportState.scissorCount = 1;
    viewportState.pViewports = nullptr; // Dynamic
    viewportState.pScissors = nullptr; // Dynamic

	vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eNone;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

	vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.sampleShadingEnable = VK_FALSE; // When enabled, perform shading per sample instead of per pixel (more expensive, essentially FSAA)
    multisampling.minSampleShading = 1.0f; // Minimum percent of samples to run full shading for when sampleShadingEnable is enabled (1.0f to run for all)
    multisampling.pSampleMask = nullptr; // Normally one bit for each sample: e.g. 0x0000000F to enable all samples in a 4-sample setup
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    vk::PipelineDepthStencilStateCreateInfo depthStencil = { };
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | 
		vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB 
		| vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[ 0 ] = 0.0f;
    colorBlending.blendConstants[ 1 ] = 0.0f;
    colorBlending.blendConstants[ 2 ] = 0.0f;
    colorBlending.blendConstants[ 3 ] = 0.0f;

	vk::GraphicsPipelineCreateInfo pipelineInfo;
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

    _dynamicStates[ 0 ] = vk::DynamicState::eViewport;
    _dynamicStates[ 1 ] = vk::DynamicState::eScissor;
    _dynamicStates[ 2 ] = vk::DynamicState::eStencilReference;

    uint32_t numDynamicStates = sizeof( _dynamicStates ) / sizeof( _dynamicStates[ 0 ] );
    assert( numDynamicStates == 3 );

    _dynamicStateInfo.dynamicStateCount = numDynamicStates;
    _dynamicStateInfo.pDynamicStates = _dynamicStates;


    pipelineInfo.pDynamicState = &_dynamicStateInfo;

    if ( vkCreateGraphicsPipelines( logicalDevice, VK_NULL_HANDLE, 1, 
      &pipelineInfo, nullptr, &graphicsPipeline ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create graphics pipeline!" );
    }


    delete fragShaderModule;
    delete vertShaderModule;


    // COMMAND POOL
    vk::CommandPoolCreateInfo poolCI;
    poolCI.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolCI.queueFamilyIndex = _getPresentDevice( )->getQueueFamily(
      GpuQueueType::GPUT_GRAPHICS );
	commandPool = logicalDevice.createCommandPool(poolCI);


    {
      vk::Format depthFormat = findDepthFormat( );
      // Depth texture
      createImage( _renderWindow->_swapChain->getWidth( ),
        _renderWindow->_swapChain->getHeight( ), depthFormat,
        vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal, depthStencilTex.image, depthStencilTex.mem );
      depthStencilTex.view = createImageView( depthStencilTex.image, depthFormat,
        vk::ImageAspectFlagBits::eDepth);

      transitionImageLayout( depthStencilTex.image, depthFormat, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal);
    }




    // FRAMEBUFFERS
    swapChainFramebuffers.resize( _renderWindow->_swapChain->swapChainImageViews.size( ) );

    for ( size_t i = 0; i < _renderWindow->_swapChain->swapChainImageViews.size( ); ++i )
    {
      std::array<vk::ImageView, 2> attachments = {
        _renderWindow->_swapChain->swapChainImageViews[ i ],
        depthStencilTex.view
      };

      vk::FramebufferCreateInfo framebufferInfo;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>( attachments.size( ) );
      framebufferInfo.pAttachments = attachments.data( );
      framebufferInfo.width = _renderWindow->_swapChain->getWidth( );
      framebufferInfo.height = _renderWindow->_swapChain->getHeight( );
      framebufferInfo.layers = 1;

	  swapChainFramebuffers[i] = logicalDevice.createFramebuffer(framebufferInfo);
    }

    auto createBuffer = [ &]( vk::DeviceSize size, vk::BufferUsageFlags usage,
      vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
		vk::DeviceMemory& bufferMemory )
    {
		vk::BufferCreateInfo bufferInfo;
      bufferInfo.size = size;
      bufferInfo.usage = usage;
      bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	  buffer = logicalDevice.createBuffer(bufferInfo);

	  vk::MemoryRequirements memRequirements = logicalDevice.getBufferMemoryRequirements(buffer);

      bufferMemory = _getPresentDevice( )->allocateMemReqMemory(
        memRequirements,
        properties );

	  logicalDevice.bindBufferMemory(buffer, bufferMemory, 0);
    };

    auto copyBufferToImage = [ & ]( VkBuffer buffer, VkImage image, 
      uint32_t width, uint32_t height )
    {
      vk::CommandBuffer commandBuffer = beginSingleTimeCommands( );

	  vk::BufferImageCopy region = { };
      region.bufferOffset = 0;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;

      region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      region.imageSubresource.mipLevel = 0;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = 1;

      region.imageOffset = { 0, 0, 0 };
      region.imageExtent = {
        width,
        height,
        1
      };

	  commandBuffer.copyBufferToImage(
        buffer,
        image,
        vk::ImageLayout::eTransferDstOptimal,
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
      vk::DeviceSize imageSize = texWidth * texHeight * 4;

      if ( !pixels )
      {
        throw std::runtime_error( "failed to load texture image!" );
      }

	  vk::Buffer stagingBuffer;
	  vk::DeviceMemory stagingBufferMemory;
      createBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory );

      void* data;
	  logicalDevice.mapMemory( stagingBufferMemory, 0, imageSize, 0, &data );
      memcpy( data, pixels, static_cast<size_t>( imageSize ) );
	  logicalDevice.unmapMemory( stagingBufferMemory );

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

      SamplerStateDesc samplerDesc;
      samplerDesc.mipFilter = FilterOptions::ANISOTROPIC;
      samplerDesc.maxAniso = 1;

      textureSampler = new VulkanSamplerState( samplerDesc );
    }

    auto copyBuffer = [ &] ( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size )
    {
      vk::CommandBuffer commandBuffer = beginSingleTimeCommands( );

	  vk::BufferCopy copyRegion = { };
      copyRegion.size = size;
      vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

      endSingleTimeCommands( commandBuffer );
    };

    // Vertex buffer creation
    {
		vk::DeviceSize bufferSize = sizeof( vertices[ 0 ] ) * vertices.size( );

	  vk::Buffer stagingBuffer;
	  vk::DeviceMemory stagingBufferMemory;
      
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
      std::array<VkDescriptorPoolSize, 2> poolSizes;
      poolSizes[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSizes[ 0 ].descriptorCount = 1;
      poolSizes[ 1 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      poolSizes[ 1 ].descriptorCount = 1;

      VkDescriptorPoolCreateInfo poolInfo;
      poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolInfo.pNext = nullptr;
      poolInfo.flags = 0;
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
      clearValues[ 0 ].color = { 0.2f, 0.3f, 0.3f, 1.0f };
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
          
          
          pushConstants[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
          pushConstants[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
          pushConstants[2] = glm::vec4(0.0f,0.0f, 1.0, 1.5f);


          // Submit via push constant (rather than a UBO)
          vkCmdPushConstants(
            commandBuffers[i],
            pipelineLayout,
            vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(pushConstants),
            pushConstants.data());

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
    semaphores.presentComplete = new VulkanSemaphore( _getPresentDevice( ) );
    semaphores.renderComplete = new VulkanSemaphore( _getPresentDevice( ) );
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
      }

      if ( !layerFound )
      {
        return false;
      }
    }

    return true;
  }
}