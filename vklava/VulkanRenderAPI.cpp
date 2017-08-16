#include "VulkanRenderAPI.h"

#include <assert.h>

#include <functional>
#include <fstream>

#include <sstream>
#include "routes.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace vklava
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
    glfwSetWindowSizeCallback( _window, VulkanRenderAPI::onWindowResized );
  }

  VulkanRenderAPI::~VulkanRenderAPI( void )
  {
  }

  void VulkanRenderAPI::cleanupSwapChain( void )
  {
    VkDevice logicalDevice = getPresentDevice( )->getLogical( );
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
    VkDevice logicalDevice = getPresentDevice( )->getLogical( );
    getPresentDevice( )->waitIdle( );

    vkDestroySampler( logicalDevice, textureSampler, nullptr );
    vkDestroyImageView( logicalDevice, textureImageView, nullptr );

    vkDestroyImage( logicalDevice, textureImage, nullptr );
    getPresentDevice( )->freeMemory( textureImageMemory );

    vkDestroyDescriptorPool( logicalDevice, descriptorPool, nullptr );
    vkDestroyDescriptorSetLayout( logicalDevice, descriptorSetLayout, nullptr );

    vkDestroyBuffer( logicalDevice, uniformBuffer, nullptr );
    getPresentDevice( )->freeMemory( uniformBufferMemory );

    vkDestroyBuffer( logicalDevice, vertexBuffer, nullptr );
    getPresentDevice( )->freeMemory( vertexBufferMemory );

    vkDestroyBuffer( logicalDevice, indexBuffer, nullptr );
    getPresentDevice( )->freeMemory( indexBufferMemory );

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

    std::shared_ptr<VulkanDevice> presentDevice = this->getPresentDevice( );
    VkPhysicalDevice physicalDevice = presentDevice->getPhysical( );

    // TODO: MOVE TO ANOTHER ZONE
    std::function<std::vector<char>( const std::string& )> readFile =
      [ &] ( const std::string& filename )
    {
      std::ifstream file( filename, std::ios::ate | std::ios::binary );

      if ( !file.is_open( ) )
      {
        throw std::runtime_error( "failed to open file!" );
      }

      size_t fileSize = ( size_t ) file.tellg( );
      std::vector<char> buffer( fileSize );

      file.seekg( 0 );
      file.read( buffer.data( ), fileSize );

      file.close( );

      return buffer;
    };

    std::function<VkShaderModule( const std::vector<char>& code )>
      createShaderModule = [ &] ( const std::vector<char>& code )
    {
      VkShaderModuleCreateInfo createInfo = { };
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      createInfo.codeSize = code.size( );
      createInfo.pCode = reinterpret_cast< const uint32_t* >( code.data( ) );

      VkShaderModule shaderModule;
      if ( vkCreateShaderModule( getPresentDevice( )->getLogical( ),
        &createInfo, nullptr, &shaderModule ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create shader module!" );
      }

      return shaderModule;
    };

    auto vertShaderCode = readFile( VKLAVA_EXAMPLES_RESOURCES_ROUTE +
      std::string( "/vert.spv" ) );
    auto fragShaderCode = readFile( VKLAVA_EXAMPLES_RESOURCES_ROUTE +
      std::string( "/frag.spv" ) );

    VkShaderModule vertShaderModule = createShaderModule( vertShaderCode );
    VkShaderModule fragShaderModule = createShaderModule( fragShaderCode );

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
      vertShaderStageInfo, fragShaderStageInfo
    };



    // RENDER PASSES
    VkDevice logicalDevice = getPresentDevice( )->getLogical( );


    VkAttachmentDescription colorAttachment = { };
    colorAttachment.format = _renderWindow->_colorFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = { };
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = { };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = { };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if ( vkCreateRenderPass( logicalDevice, &renderPassInfo,
      nullptr, &renderPass ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create render pass!" );
    }

    // UBO DESCRIPTOR
    VkDescriptorSetLayoutBinding uboLayoutBinding = { };
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = { };
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo = { };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>( bindings.size( ) );
    layoutInfo.pBindings = bindings.data( );

    if ( vkCreateDescriptorSetLayout( logicalDevice, &layoutInfo, 
      nullptr, &descriptorSetLayout ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create descriptor set layout!" );
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


    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = ( float ) _renderWindow->_swapChain->getWidth( );
    viewport.height = ( float ) _renderWindow->_swapChain->getHeight( );
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = 
    {
      _renderWindow->_swapChain->getWidth( ),
      _renderWindow->_swapChain->getHeight( )
    };

    VkPipelineViewportStateCreateInfo viewportState = { };
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = { };
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if ( vkCreatePipelineLayout( logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create pipeline layout!" );
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = { };
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if ( vkCreateGraphicsPipelines( logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create graphics pipeline!" );
    }


    vkDestroyShaderModule( logicalDevice, fragShaderModule, nullptr );
    vkDestroyShaderModule( logicalDevice, vertShaderModule, nullptr );






    // FRAMEBUFFERS
    swapChainFramebuffers.resize( _renderWindow->_swapChain->swapChainImageViews.size( ) );

    for ( size_t i = 0; i < _renderWindow->_swapChain->swapChainImageViews.size( ); i++ )
    {
      VkImageView attachments[] = {
        _renderWindow->_swapChain->swapChainImageViews[ i ]
      };

      VkFramebufferCreateInfo framebufferInfo = { };
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = _renderWindow->_swapChain->getWidth( );
      framebufferInfo.height = _renderWindow->_swapChain->getHeight( );
      framebufferInfo.layers = 1;

      if ( vkCreateFramebuffer( logicalDevice,
        &framebufferInfo, nullptr, &swapChainFramebuffers[ i ] ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create framebuffer!" );
      }
    }

    // COMMAND POOL
    VkCommandPoolCreateInfo poolCI;
    poolCI.pNext = nullptr;
    poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.queueFamilyIndex = getPresentDevice( )->getQueueFamily(
      GpuQueueType::GPUT_GRAPHICS );
    if ( vkCreateCommandPool( logicalDevice, &poolCI, nullptr,
      &commandPool ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create command pool!" );
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

      auto findMemoryType = [ &] ( uint32_t typeFilter, VkMemoryPropertyFlags properties )
      {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );

        for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
        {
          if ( ( typeFilter & ( 1 << i ) ) &&
            ( memProperties.memoryTypes[ i ].propertyFlags & properties ) == properties )
          {
            return i;
          }
        }

        throw std::runtime_error( "failed to find suitable memory type!" );
      };

      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements( logicalDevice, buffer, &memRequirements );

      bufferMemory = getPresentDevice( )->allocateMemReqMemory(
        memRequirements,
        properties );

      vkBindBufferMemory( logicalDevice, buffer, bufferMemory, 0 );
    };

    auto beginSingleTimeCommands = [ & ]( )
    {
      VkCommandBufferAllocateInfo allocInfo = { };
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool = commandPool;
      allocInfo.commandBufferCount = 1;

      VkCommandBuffer commandBuffer;
      vkAllocateCommandBuffers( logicalDevice, &allocInfo, &commandBuffer );

      VkCommandBufferBeginInfo beginInfo = { };
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      vkBeginCommandBuffer( commandBuffer, &beginInfo );

      return commandBuffer;
    };

    auto endSingleTimeCommands = [ & ] ( VkCommandBuffer commandBuffer )
    {
      vkEndCommandBuffer( commandBuffer );

      VkSubmitInfo submitInfo = { };
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffer;

      VulkanQueue* graphicsQueue = getPresentDevice( )->getQueue(
        GpuQueueType::GPUT_GRAPHICS, 0 );
      vkQueueSubmit( graphicsQueue->getQueue( ), 1, &submitInfo, VK_NULL_HANDLE );
      graphicsQueue->waitIdle( );

      vkFreeCommandBuffers( logicalDevice, commandPool, 1, &commandBuffer );
    };

    auto transitionImageLayout = [ & ]( VkImage image, VkFormat format, 
      VkImageLayout oldLayout, VkImageLayout newLayout )
    {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands( );

      VkImageMemoryBarrier barrier = { };
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.oldLayout = oldLayout;
      barrier.newLayout = newLayout;

      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

      barrier.image = image;
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      barrier.subresourceRange.baseMipLevel = 0;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.baseArrayLayer = 0;
      barrier.subresourceRange.layerCount = 1;

      VkPipelineStageFlags sourceStage;
      VkPipelineStageFlags destinationStage;

      if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED 
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
      {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      }
      else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
      {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      }
      else
      {
        throw std::invalid_argument( "unsupported layout transition!" );
      }

      vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
      );

      endSingleTimeCommands( commandBuffer );
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
        std::string( "/texture.jpg" ) ).c_str( ),
        &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
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

      auto createImage = [ &]( uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, VkImage& image,
        VkDeviceMemory& imageMemory )
      {
        VkImageCreateInfo imageInfo = { };
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.flags = 0;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>( texWidth );
        imageInfo.extent.height = static_cast<uint32_t>( texHeight );
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        //
        // -VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major order like our pixels array
        // -VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        // 
        // -VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
        // -VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        if ( vkCreateImage( logicalDevice, &imageInfo, nullptr, &textureImage ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to create image!" );
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements( logicalDevice, image, &memRequirements );

        imageMemory = getPresentDevice( )->allocateMemReqMemory( memRequirements, properties );

        vkBindImageMemory( logicalDevice, image, imageMemory, 0 );
      };

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
      getPresentDevice( )->freeMemory( stagingBufferMemory );


      auto createImageView = [ & ] ( VkImage image, VkFormat format )
      {
        VkImageViewCreateInfo viewInfo = { };
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if ( vkCreateImageView( logicalDevice, &viewInfo, nullptr, &imageView ) 
          != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to create texture image view!" );
        }

        return imageView;
      };

      textureImageView = createImageView( textureImage, VK_FORMAT_R8G8B8A8_UNORM );

      // TEXTURE SAMPLER
      VkSamplerCreateInfo samplerInfo = { };
      samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      samplerInfo.pNext = nullptr;
      samplerInfo.flags = 0;
      samplerInfo.magFilter = VK_FILTER_LINEAR;
      samplerInfo.minFilter = VK_FILTER_LINEAR;
      samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      //samplerInfo.anisotropyEnable = VK_TRUE;
      //samplerInfo.maxAnisotropy = 16;
      samplerInfo.anisotropyEnable = VK_FALSE;
      samplerInfo.maxAnisotropy = 1;

      samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      samplerInfo.unnormalizedCoordinates = VK_FALSE;
      samplerInfo.compareEnable = VK_FALSE;
      samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
      samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

      if ( vkCreateSampler( logicalDevice, &samplerInfo, nullptr, &textureSampler ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create texture sampler!" );
      }
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
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory );

      copyBuffer( stagingBuffer, vertexBuffer, bufferSize );

      vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
      vkFreeMemory( logicalDevice, stagingBufferMemory, nullptr );
    }

    // Index buffer creation
    {
      VkDeviceSize bufferSize = sizeof( indices[ 0 ] ) * indices.size( );

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory );

      void* data;
      vkMapMemory( logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data );
      memcpy( data, indices.data( ), ( size_t ) bufferSize );
      vkUnmapMemory( logicalDevice, stagingBufferMemory );

      createBuffer( bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory );

      copyBuffer( stagingBuffer, indexBuffer, bufferSize );

      vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
      vkFreeMemory( logicalDevice, stagingBufferMemory, nullptr );
    }

    // UBO buffer creation
    {
      VkDeviceSize bufferSize = sizeof( UniformBufferObject );
      createBuffer( bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBuffer, uniformBufferMemory );
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

      VkDescriptorBufferInfo bufferInfo = { };
      bufferInfo.buffer = uniformBuffer;
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof( UniformBufferObject );

      VkDescriptorImageInfo imageInfo = { };
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = textureImageView;
      imageInfo.sampler = textureSampler;

      std::array<VkWriteDescriptorSet, 2> descriptorWrites = { };

      descriptorWrites[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[ 0 ].dstSet = descriptorSet;
      descriptorWrites[ 0 ].dstBinding = 0;
      descriptorWrites[ 0 ].dstArrayElement = 0;
      descriptorWrites[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[ 0 ].descriptorCount = 1;
      descriptorWrites[ 0 ].pBufferInfo = &bufferInfo;

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

      VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
      renderPassBeginInfo.clearValueCount = 1;
      renderPassBeginInfo.pClearValues = &clearColor;

      vkCmdBeginRenderPass( cmd, &renderPassBeginInfo,
        VK_SUBPASS_CONTENTS_INLINE );

        /*// Basic drawing commands
        vkCmdBindPipeline( commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, 
          graphicsPipeline );

        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers( cmd, 0, 1, vertexBuffers,
          offsets );

        vkCmdDraw( cmd, static_cast< uint32_t >(
          vertices.size( ) ), 1, 0, 0 );*/

        vkCmdBindPipeline( commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, 
          graphicsPipeline );

        VkBuffer vertexBuffers[ ] = { vertexBuffer };
        VkDeviceSize offsets[ ] = { 0 };
        vkCmdBindVertexBuffers( commandBuffers[ i ], 0, 1, vertexBuffers, offsets );

        vkCmdBindDescriptorSets( commandBuffers[ i ], VK_PIPELINE_BIND_POINT_GRAPHICS, 
          pipelineLayout, 0, 1, &descriptorSet, 0, nullptr );

        vkCmdBindIndexBuffer( commandBuffers[ i ], indexBuffer, 0, 
          VK_INDEX_TYPE_UINT16 );

        vkCmdDrawIndexed( commandBuffers[ i ], 
          static_cast<uint32_t>( indices.size( ) ), 1, 0, 0, 0 );

      vkCmdEndRenderPass( commandBuffers[ i ] );

      if ( vkEndCommandBuffer( commandBuffers[ i ] ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to record command buffer!" );
      }
      ++i;
    }

    // SEMAPHORES
    imageAvailableSemaphore = new VulkanSemaphore( getPresentDevice( ) );
    renderFinishedSemaphore = new VulkanSemaphore( getPresentDevice( ) );
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