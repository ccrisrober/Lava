/**
 * Lava.
 * File: ShadowMappingByJGM.cpp
 * Author: Juan Guerrero Martín.
 * Brief: Following The Khronos Group Inc. tutorial (https://vulkan-tutorial.com/).
 */

// Lava.
#include <lava/lava.h>
#include <routes.h>

// glfw.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// glm.
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// stbi.
#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

// std.
#include <array>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional> // Maybe not needed.
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

/** GLFW. **/
const int WIDTH = 800;
const int HEIGHT = 600;

/** Validation layers. **/

// Using validation layers only if debugging.
const std::vector< const char* > validationLayers =
{
  "VK_LAYER_LUNARG_standard_validation"
};

// Device extensions.
const std::vector< const char* > deviceExtensions =
{
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// NDEBUG C++ macro that stands for "no debug".
/**/
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
/**/
// Disabling validation layers.
// const bool enableValidationLayers = false;

// An extension function has an address (vkGetInstanceProcAddr).
VkResult CreateDebugReportCallbackEXT( VkInstance instance,
                                       const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator,
                                       VkDebugReportCallbackEXT* pCallback )
{
  auto func = ( PFN_vkCreateDebugReportCallbackEXT )
              vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
  return (func != nullptr) ?
         func( instance, pCreateInfo, pAllocator, pCallback )
         : VK_ERROR_EXTENSION_NOT_PRESENT;
}

// This function should be static or an outsider.
void DestroyDebugReportCallbackEXT( VkInstance instance,
                                    VkDebugReportCallbackEXT callback,
                                    const VkAllocationCallbacks* pAllocator )
{
  auto func = ( PFN_vkDestroyDebugReportCallbackEXT )
              vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
  if (func != nullptr)
  {
    func( instance, callback, pAllocator );
  }
}

/** Queue-related. **/

struct QueueFamilyIndices
{
  int graphicsFamily = -1;
  int presentFamily = -1;

  bool isComplete( void )
  {
    return graphicsFamily >= 0 && presentFamily >= 0;
  }
};

/** Surface-related. **/

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector< VkSurfaceFormatKHR > formats;
  std::vector< VkPresentModeKHR > presentModes;
};

/** Uniforms. **/

// Descriptors.
struct UniformBufferObject
{
  glm::mat4 lightView;
  glm::mat4 cameraView;
  glm::mat4 proj;
};

/** Meshes **/

// Mesh 1: Bunny.
std::string bunnyMeshPath = LAVA_EXAMPLES_MESHES_ROUTE + std::string("bunny.obj_");
lava::extras::ModelImporter bunnyModel( bunnyMeshPath );
lava::extras::Mesh bunnyMesh = bunnyModel._meshes[ 0 ];

// Mesh 2: Table (Cube).
std::string tableMeshPath = LAVA_EXAMPLES_MESHES_ROUTE + std::string("cube.obj_");
lava::extras::ModelImporter tableModel( tableMeshPath );
lava::extras::Mesh tableMesh = tableModel._meshes[ 0 ];

/** Shaders. They must be in *.spv Vulkan format. **/
std::string shadowMappingFirstPassByJGMVertexShader( "/home/jguerrero/opt/Lava/spvs/JGM/shadowMappingFirstPassByJGM_vert.spv" );
std::string shadowMappingSecondPassByJGMVertexShader( "/home/jguerrero/opt/Lava/spvs/JGM/shadowMappingSecondPassByJGM_vert.spv" );
std::string shadowMappingSecondPassByJGMFragmentShader( "/home/jguerrero/opt/Lava/spvs/JGM/shadowMappingSecondPassByJGM_frag.spv" );

class HelloTriangleApplication
{

  public:
    void run( void )
    {
      initWindow( );
      initVulkan( );
      mainLoop( );
      cleanup( );
    }

  private:

    /** Attributes. **/

    // GLFW.
    GLFWwindow* window;

    // Instance.
    VkInstance instance;
    VkDebugReportCallbackEXT callback;
    VkSurfaceKHR surface;

    // Device.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // It will be destroyed with VkInstance.
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    // SwapChain.
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector< VkImage > swapChainImages;
    std::vector< VkImageView > swapChainImageViews;

    // RenderPass.
    VkRenderPass firstRenderPass;
    VkRenderPass secondRenderPass;

    // PipelineLayout (DescriptorSetLayout and PushConstantRange).
    std::vector< VkPushConstantRange > pushConstantRangesForFirstPass;
    std::vector< VkPushConstantRange > pushConstantRangesForSecondPass;

    VkDescriptorSetLayout descriptorSetLayoutForFirstPass;
    VkDescriptorSetLayout descriptorSetLayoutForSecondPass;

    VkPipelineLayout pipelineLayoutForFirstPass;
    VkPipelineLayout pipelineLayoutForSecondPass;

    // Pipeline.
    VkPipeline graphicsPipelineForFirstPass;
    VkPipeline graphicsPipelineForSecondPass;

    // CommandPool.
    VkCommandPool commandPool;

    // Attachment.
    VkImage shadowMapImage;
    VkDeviceMemory shadowMapImageMemory;
    VkImageView shadowMapImageView;
    VkSampler shadowMapSampler;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    /**
     * Framebuffer.
     */
    std::vector< VkFramebuffer > swapChainFramebuffersForFirstPass;
    std::vector< VkFramebuffer > swapChainFramebuffersForSecondPass;

    /**
     * Buffer.
     */
    VkBuffer bunnyVertexBuffer;
    VkDeviceMemory bunnyVertexBufferMemory;
    VkBuffer bunnyIndexBuffer;
    VkDeviceMemory bunnyIndexBufferMemory;

    VkBuffer tableVertexBuffer;
    VkDeviceMemory tableVertexBufferMemory;
    VkBuffer tableIndexBuffer;
    VkDeviceMemory tableIndexBufferMemory;

    glm::vec3 defaultLightPosition = glm::vec3( 0.0f, 8.0f, 3.0f );
    glm::vec3 defaultCameraPosition = glm::vec3( 7.0f, 25.0f, -7.0f );

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;

    VkDescriptorPool descriptorPoolForFirstPass;
    VkDescriptorPool descriptorPoolForSecondPass;

    VkDescriptorSet descriptorSetForFirstPass;
    VkDescriptorSet descriptorSetForSecondPass;

    /**
     * CommandBuffer.
     */
    std::vector< VkCommandBuffer > commandBuffers;

    /**
     * Synchronization.
     */
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    /** Basic functions. **/

    void initWindow( void )
    {
      glfwInit( );

      // Not creating an OGL context.
      glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
      // Resizing is special in Vulkan. May require recreateSwapChain( ).
      glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

      // 4th: specific monitor. 5th: OGL relevant.
      window = glfwCreateWindow( WIDTH, HEIGHT, "Vulkan", nullptr, nullptr );

      glfwSetWindowUserPointer( window, this );
      glfwSetWindowSizeCallback( window, HelloTriangleApplication::onWindowResized );
    }

    void initVulkan( void )
    {
      /**
       * Instance (with an application).
       **/
      createInstance( );
      // Validation layers (debugging).
      setupDebugCallback( );
      // Creation of a surface out of an instance and a window.
      createSurface( );

      /**
       * Device (physical and logical).
       */
      pickPhysicalDevice( );
      createLogicalDevice( );

      /**
       * SwapChain (queue of images for rendering purposes).
       */
      createSwapChain( );
      createSwapChainImageViews( );

      /**
       * RenderPass.
       */
      createFirstRenderPass( );
      createSecondRenderPass( );

      /**
       * PipelineLayout.
       */
      createDescriptorSetLayoutForFirstPass( );
      createDescriptorSetLayoutForSecondPass( );
      createPushConstantRangesForFirstPass( );
      createPushConstantRangesForSecondPass( );

      /**
       * Pipeline.
       */
      createGraphicsPipelineForFirstPass( );
      createGraphicsPipelineForSecondPass( );

      /**
       * CommandPool (commands needed for a lot of operations).
       *   - copyBuffer.
       *   - copyBufferToImage.
       *   - transitionImageLayout.
       */
      createCommandPool( );

      /**
       * Attachment.
       */
      createShadowMapImageAndImageView( );
      createShadowMapSampler( );
      createDepthResources( );

      /**
       * Framebuffer (RenderPass and Attachment).
       */
      createFramebuffersForFirstPass( );
      createFramebuffersForSecondPass( );

      /**
       * Buffer.
       */
      createVertexBuffers( );
      createIndexBuffers( );
      createUniformBuffer( );
      initUniformBuffer( );

      createDescriptorPoolForFirstPass( );
      createDescriptorPoolForSecondPass( );
      createDescriptorSetForFirstPass( );
      createDescriptorSetForSecondPass( );
      updateDescriptorSetForFirstPass( );
      updateDescriptorSetForSecondPass( );

      /**
       * CommandBuffer.
       */
      createCommandBuffers( );

      /**
       * Synchronization.
       */
      createSemaphores( );
    }

    void mainLoop( void )
    {
      while( !glfwWindowShouldClose( window ) )
      {
        glfwPollEvents( );

        //updateUniformBuffer( );
        drawFrame( );
      }
      vkDeviceWaitIdle(device);
    }

    void cleanupSwapChain( void )
    {
      /**
       * Attachment.
       */
      vkDestroySampler( device, shadowMapSampler, nullptr );
      vkDestroyImageView( device, shadowMapImageView, nullptr );
      vkDestroyImage( device, shadowMapImage, nullptr );
      vkFreeMemory( device, shadowMapImageMemory, nullptr );

      vkDestroyImageView( device, depthImageView, nullptr );
      vkDestroyImage( device, depthImage, nullptr );
      vkFreeMemory( device, depthImageMemory, nullptr );

      /**
       * Framebuffer.
       */
      for( size_t i = 0; i < swapChainFramebuffersForFirstPass.size( ); i++ )
      {
        vkDestroyFramebuffer( device, swapChainFramebuffersForFirstPass[i], nullptr );
      }
      for( size_t i = 0; i < swapChainFramebuffersForSecondPass.size( ); i++ )
      {
        vkDestroyFramebuffer( device, swapChainFramebuffersForSecondPass[i], nullptr );
      }

      /**
       * CommandBuffer.
       */
      vkFreeCommandBuffers( device, commandPool,
                            static_cast< uint32_t >( commandBuffers.size( ) ), commandBuffers.data( ) );

      /**
       * Pipeline.
       */
      vkDestroyPipeline( device, graphicsPipelineForFirstPass, nullptr );
      vkDestroyPipeline( device, graphicsPipelineForSecondPass, nullptr );

      /**
       * PipelineLayout.
       */
      vkDestroyPipelineLayout( device, pipelineLayoutForFirstPass, nullptr );
      vkDestroyPipelineLayout( device, pipelineLayoutForSecondPass, nullptr );
      vkDestroyDescriptorSetLayout( device, descriptorSetLayoutForFirstPass, nullptr );
      vkDestroyDescriptorSetLayout( device, descriptorSetLayoutForSecondPass, nullptr );
      // PushConstantRanges have no a destroy function.

      /**
       * RenderPass.
       */
      vkDestroyRenderPass( device, firstRenderPass, nullptr );
      vkDestroyRenderPass( device, secondRenderPass, nullptr );

      /**
       * SwapChain.
       */
      for( size_t i = 0; i < swapChainImageViews.size( ); i++ )
      {
        vkDestroyImageView( device, swapChainImageViews[i], nullptr );
      }
      vkDestroySwapchainKHR( device, swapChain, nullptr );
    }

    void cleanup( void )
    {
      /**
       * SwapChain, RenderPass, PipelineLayout, Pipeline,
       * Attachment, Framebuffer, CommandBuffer.
       **/
      cleanupSwapChain( );

      /**
       * Buffer.
       */
      vkDestroyDescriptorPool( device, descriptorPoolForFirstPass, nullptr );
      vkDestroyDescriptorPool( device, descriptorPoolForSecondPass, nullptr );
      // Descriptor sets destroyed by the descriptor pools.

      vkDestroyBuffer( device, uniformBuffer, nullptr );
      vkFreeMemory( device, uniformBufferMemory, nullptr );

      vkDestroyBuffer( device, bunnyVertexBuffer, nullptr );
      vkFreeMemory( device, bunnyVertexBufferMemory, nullptr );
      vkDestroyBuffer( device, bunnyIndexBuffer, nullptr );
      vkFreeMemory( device, bunnyIndexBufferMemory, nullptr );

      vkDestroyBuffer( device, tableVertexBuffer, nullptr );
      vkFreeMemory( device, tableVertexBufferMemory, nullptr );
      vkDestroyBuffer( device, tableIndexBuffer, nullptr );
      vkFreeMemory( device, tableIndexBufferMemory, nullptr );

      /**
       * Synchronization.
       */
      vkDestroySemaphore( device, renderFinishedSemaphore, nullptr );
      vkDestroySemaphore( device, imageAvailableSemaphore, nullptr );

      /**
       * CommandPool.
       */
      vkDestroyCommandPool( device, commandPool, nullptr );

      //-----------------------------------------------------------------------

      /**
       * Device.
       */
      // All the queues (graphics and present) will be destroyed with the device.
      vkDestroyDevice( device, nullptr );
      // The physical device will be destroyed with the instance.

      /**
       * Instance.
       **/
      DestroyDebugReportCallbackEXT( instance, callback, nullptr );
      vkDestroySurfaceKHR( instance, surface, nullptr );
      vkDestroyInstance( instance, nullptr );

      // GLFW (window).
      glfwDestroyWindow( window );
      glfwTerminate( );
    }

    /** General functions. **/

    static void onWindowResized( GLFWwindow* window, int width, int height )
    {
      if( width == 0 || height == 0 ) return;

      HelloTriangleApplication* app =
        reinterpret_cast< HelloTriangleApplication* >( glfwGetWindowUserPointer( window ) );
      // Vulkan may require this.
      app->recreateSwapChain( );
    }

    void recreateSwapChain( void )
    {
      vkDeviceWaitIdle( device );

      cleanupSwapChain( );

      /**
       * SwapChain (queue of images for rendering purposes).
       */
      createSwapChain( );
      createSwapChainImageViews( );

      /**
       * RenderPass.
       */
      createFirstRenderPass( );
      createSecondRenderPass( );

      /**
       * PipelineLayout. ADDITIONAL.
       */
      createDescriptorSetLayoutForFirstPass( );
      createDescriptorSetLayoutForSecondPass( );

      /**
       * Pipeline.
       */
      createGraphicsPipelineForFirstPass( );
      createGraphicsPipelineForSecondPass( );

      /**
       * Attachment.
       */
      createShadowMapImageAndImageView( );
      createShadowMapSampler( );
      createDepthResources( );

      /**
       * Framebuffer (RenderPass and Attachment).
       */
      createFramebuffersForFirstPass( );
      createFramebuffersForSecondPass( );

      /**
       * Buffer. ADDITIONAL.
       */
      createDescriptorPoolForFirstPass( );
      createDescriptorPoolForSecondPass( );
      createDescriptorSetForFirstPass( );
      createDescriptorSetForSecondPass( );
      updateDescriptorSetForFirstPass( );
      updateDescriptorSetForSecondPass( );

      /**
       * CommandBuffer.
       */
      createCommandBuffers( );
    }

    void createInstance( void )
    {
      if( enableValidationLayers && !checkValidationLayerSupport( ) )
      {
        throw std::runtime_error( "validation layers requested, but not available!" );
      }

      // VkApplicationInfo struct. Optional.
      VkApplicationInfo appInfo = {};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = "Lava. Shadow mapping.";
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.pEngineName = "No Engine";
      appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion = VK_API_VERSION_1_0;

      // VkInstanceCreateInfo struct. Required.
      VkInstanceCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      createInfo.pApplicationInfo = &appInfo;

      // Extensions.
      auto extensions = getRequiredExtensions( );
      createInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.size( ) );
      createInfo.ppEnabledExtensionNames = extensions.data( );

      // Validation layers.
      if (enableValidationLayers)
      {
        createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size( ) );
        createInfo.ppEnabledLayerNames = validationLayers.data( );
      }
      else
      {
        createInfo.enabledLayerCount = 0;
      }

      // Parameters: creation info, custom allocator callbacks, pointer to object.
      if( vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS )
      {
        throw std::runtime_error( "Failed to create VkInstance." );
      }

      /**
      // Available extensions.
      uint32_t extensionCount = 0;
      vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

      std::vector< VkExtensionProperties > extensionProperties( extensionCount );
      vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensionProperties.data( ) );

      // Printing them.
      std::cout << "Available extensions:" << std::endl;

      for (const auto& extension : extensionProperties)
      {
        std::cout << "\t" << extension.extensionName << std::endl;
      }
      **/
    }

    void setupDebugCallback( void )
    {
      if( !enableValidationLayers ) return;

      VkDebugReportCallbackCreateInfoEXT createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
      createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
      createInfo.pfnCallback = debugCallback;

      if( CreateDebugReportCallbackEXT( instance, &createInfo, nullptr, &callback ) != VK_SUCCESS )
      {
        throw std::runtime_error("failed to set up debug callback!");
      }
    }

    void createSurface( void )
    {
      // That is how a surface for Windows OS would be created behind the scenes.
      /**
      VkWin32SurfaceCreateInfoKHR createInfo;
      createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
      createInfo.hwnd = glfwGetWin32Window( window );
      createInfo.hinstance = GetModuleHandle( nullptr );

      auto CreateWin32SurfaceKHR =
        (PFN_vkCreateWin32SurfaceKHR) vkGetInstanceProcAddr( instance, "vkCreateWin32SurfaceKHR" );

      if( !CreateWin32SurfaceKHR ||
          CreateWin32SurfaceKHR( instance, &createInfo, nullptr, &surface ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create window surface!" );
      }
      **/

      // Easy and cross-platform way.
      if( glfwCreateWindowSurface( instance, window, nullptr, &surface ) != VK_SUCCESS )
      {
        throw std::runtime_error("failed to create window surface!");
      }
    }

    void pickPhysicalDevice( void )
    {
      uint32_t deviceCount = 0;
      vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );

      if( deviceCount == 0 )
      {
        throw std::runtime_error( "failed to find GPUs with Vulkan support!" );
      }

      std::vector< VkPhysicalDevice > devices( deviceCount );
      vkEnumeratePhysicalDevices( instance, &deviceCount, devices.data( ) );

      // Checking if all GPUs are suitable for specific operations.
      // Get the first that passes the test.
      for( const auto& device : devices )
      {
        if( isDeviceSuitable( device ) )
        {
          physicalDevice = device;
          break;
        }
      }

      if( physicalDevice == VK_NULL_HANDLE )
      {
        throw std::runtime_error( "failed to find a suitable GPU!" );
      }
    }

    void createLogicalDevice( void )
    {
      QueueFamilyIndices indices = findQueueFamilies( physicalDevice );

      // Multiple VkDeviceQueueCreateInfo.
      std::vector< VkDeviceQueueCreateInfo > queueCreateInfos;
      std::set< int > uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

      float queuePriority = 1.0f;
      for( int queueFamily : uniqueQueueFamilies )
      {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back( queueCreateInfo );
      }

      // VkPhysicalDeviceFeatures. What features are going to be used.
      VkPhysicalDeviceFeatures deviceFeatures = {};

      // Only for textures. It is an expensive operation.
      // deviceFeatures.samplerAnisotropy = VK_TRUE;

      // Actually creating the logical device.
      VkDeviceCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

      createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size( ) );
      createInfo.pQueueCreateInfos = queueCreateInfos.data( );

      createInfo.pEnabledFeatures = &deviceFeatures;

      // Device extension: VK_KHR_swapchain.
      createInfo.enabledExtensionCount = static_cast< uint32_t >( deviceExtensions.size( ) );
      createInfo.ppEnabledExtensionNames = deviceExtensions.data( );

      if( enableValidationLayers )
      {
        createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size( ) );
        createInfo.ppEnabledLayerNames = validationLayers.data( );
      }
      else
      {
        createInfo.enabledLayerCount = 0;
      }

      if( vkCreateDevice( physicalDevice, &createInfo, nullptr, &device ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create logical device!" );
      }

      // Getting VkQueue(s) from the logical device.
      vkGetDeviceQueue( device, indices.graphicsFamily, 0, &graphicsQueue );
      vkGetDeviceQueue( device, indices.presentFamily, 0, &presentQueue );
    }

    void createSwapChain( void )
    {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport( physicalDevice );

      VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupport.formats );
      VkPresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupport.presentModes );
      VkExtent2D extent = chooseSwapExtent( swapChainSupport.capabilities );

      // Trying (+1) to implement triple buffering.
      uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
      // maxImageCount = 0 means no limits.
      if( swapChainSupport.capabilities.maxImageCount > 0 &&
          imageCount > swapChainSupport.capabilities.maxImageCount )
      {
        imageCount = swapChainSupport.capabilities.maxImageCount;
      }

      VkSwapchainCreateInfoKHR createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      createInfo.surface = surface;
      createInfo.minImageCount = imageCount;
      createInfo.imageFormat = surfaceFormat.format;
      createInfo.imageColorSpace = surfaceFormat.colorSpace;
      createInfo.imageExtent = extent;
      // 1 unless stereoscopic 3D app.
      createInfo.imageArrayLayers = 1;
      // Render directly.
      createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

      // Queue issues.
      QueueFamilyIndices indices = findQueueFamilies( physicalDevice );
      uint32_t queueFamilyIndices[] =
        { (uint32_t) indices.graphicsFamily, (uint32_t) indices.presentFamily };

      if( indices.graphicsFamily != indices.presentFamily )
      {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
      }
      else
      {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
      }

      // You can apply a transformation to all images.
      createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

      // Blending with other windows. Default: no (opaque).
      createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

      createInfo.presentMode = presentMode;
      createInfo.clipped = VK_TRUE;

      // Safety pointer to current swap chain.
      createInfo.oldSwapchain = VK_NULL_HANDLE;

      if( vkCreateSwapchainKHR( device, &createInfo, nullptr, &swapChain ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create swap chain!" );
      }

      // Getting VkImage(s).
      vkGetSwapchainImagesKHR( device, swapChain, &imageCount, nullptr );
      swapChainImages.resize( imageCount );
      vkGetSwapchainImagesKHR( device, swapChain, &imageCount, swapChainImages.data( ) );

      // Getting VkFormat and VkExtent2D.
      swapChainImageFormat = surfaceFormat.format;
      swapChainExtent = extent;
    }

    void createSwapChainImageViews( void )
    {
      swapChainImageViews.resize( swapChainImages.size( ) );

      for( size_t i = 0; i < swapChainImages.size( ); i++ )
      {
        swapChainImageViews[i] = createImageView( swapChainImages[i],
                                                  swapChainImageFormat,
                                                  VK_IMAGE_ASPECT_COLOR_BIT );
      }
    }

    void createFirstRenderPass( void )
    {
      // Shadow map as the depth attachment.
      VkAttachmentDescription depthAttachment = {};
      depthAttachment.format = findDepthFormatForShadowMap( );
      depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

      VkAttachmentReference depthAttachmentRef = {};
      depthAttachmentRef.attachment = 0;
      depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      // A pass can have multiple subpasses.
      VkSubpassDescription subpass = {};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.pDepthStencilAttachment = &depthAttachmentRef;

      // Subpass dependencies.
      VkSubpassDependency firstDependency = {};
      firstDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
      firstDependency.dstSubpass = 0;
      firstDependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      firstDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
      firstDependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      firstDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

      VkSubpassDependency secondDependency = {};
      secondDependency.srcSubpass = 0;
      secondDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
      secondDependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      secondDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      secondDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      secondDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      secondDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

      std::array< VkSubpassDependency, 2 > dependencies =
        { firstDependency, secondDependency };
      std::array< VkAttachmentDescription, 1 > attachments =
        { depthAttachment };

      VkRenderPassCreateInfo renderPassInfo = {};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size( ) );
      renderPassInfo.pAttachments = attachments.data( );
      renderPassInfo.subpassCount = 1;
      renderPassInfo.pSubpasses = &subpass;
      renderPassInfo.dependencyCount = static_cast< uint32_t >( dependencies.size( ) );
      renderPassInfo.pDependencies = dependencies.data( );

      // Finally creating the render pass.
      if( vkCreateRenderPass( device, &renderPassInfo, nullptr, &firstRenderPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create render pass!" );
      }
    }

    void createSecondRenderPass( void )
    {
      VkAttachmentDescription colorAttachment = {};
      colorAttachment.format = swapChainImageFormat;
      colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

      VkAttachmentDescription depthAttachment = {};
      depthAttachment.format = findDepthFormat( );
      depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      VkAttachmentReference colorAttachmentRef = {};
      colorAttachmentRef.attachment = 0;
      colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      VkAttachmentReference depthAttachmentRef = {};
      depthAttachmentRef.attachment = 1;
      depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      // A pass can have multiple subpasses.
      VkSubpassDescription subpass = {};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &colorAttachmentRef;
      subpass.pDepthStencilAttachment = &depthAttachmentRef;

      // Subpass dependencies.
      VkSubpassDependency dependency = {};
      dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
      dependency.dstSubpass = 0;
      dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.srcAccessMask = 0;
      dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

      std::array< VkAttachmentDescription, 2 > attachments =
        { colorAttachment, depthAttachment };
      VkRenderPassCreateInfo renderPassInfo = {};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size( ) );
      renderPassInfo.pAttachments = attachments.data( );
      renderPassInfo.subpassCount = 1;
      renderPassInfo.pSubpasses = &subpass;
      renderPassInfo.dependencyCount = 1;
      renderPassInfo.pDependencies = &dependency;

      // Finally creating the render pass.
      if( vkCreateRenderPass( device, &renderPassInfo, nullptr, &secondRenderPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create render pass!" );
      }
    }

    void createDescriptorSetLayoutForFirstPass( void )
    {
      // Binding: uniform buffer.
      VkDescriptorSetLayoutBinding uboLayoutBinding = {};
      uboLayoutBinding.binding = 0;
      uboLayoutBinding.descriptorCount = 1;
      uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      std::array< VkDescriptorSetLayoutBinding, 1 > bindings =
        { uboLayoutBinding };
      VkDescriptorSetLayoutCreateInfo layoutInfo = {};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size( ) );
      layoutInfo.pBindings = bindings.data( );

      if( vkCreateDescriptorSetLayout( device, &layoutInfo, nullptr, &descriptorSetLayoutForFirstPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create descriptor set layout!" );
      }
    }

    void createDescriptorSetLayoutForSecondPass( void )
    {
      // Binding: uniform buffer.
      VkDescriptorSetLayoutBinding uboLayoutBinding = {};
      uboLayoutBinding.binding = 0;
      uboLayoutBinding.descriptorCount = 1;
      uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      // Binding: combined image sampler.
      VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
      samplerLayoutBinding.binding = 1;
      samplerLayoutBinding.descriptorCount = 1;
      samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional
      samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::array< VkDescriptorSetLayoutBinding, 2 > bindings =
        { uboLayoutBinding, samplerLayoutBinding };
      VkDescriptorSetLayoutCreateInfo layoutInfo = {};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size( ) );
      layoutInfo.pBindings = bindings.data( );

      if( vkCreateDescriptorSetLayout( device, &layoutInfo, nullptr, &descriptorSetLayoutForSecondPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create descriptor set layout!" );
      }
    }

    void createPushConstantRangesForFirstPass( void )
    {
      // 0f - 16f (modelMatrix).
      VkPushConstantRange vertexShaderPushConstant =
      { VK_SHADER_STAGE_VERTEX_BIT, 0, 16 * sizeof( float ) };

      pushConstantRangesForFirstPass.push_back( vertexShaderPushConstant );
    }

    void createPushConstantRangesForSecondPass( void )
    {
      // 0f - 16f (modelMatrix). 16f - 20f (lightPosition).
      VkPushConstantRange vertexShaderPushConstant =
      { VK_SHADER_STAGE_VERTEX_BIT, 0, 20 * sizeof( float ) };

      pushConstantRangesForSecondPass.push_back( vertexShaderPushConstant );
    }

    void createGraphicsPipelineForFirstPass( void )
    {
      // Getting shaders byte array.
      auto vertShaderCode = readFile( shadowMappingFirstPassByJGMVertexShader );

      // Here we should check that byte size is equal to file byte size.

      // VkShaderModule(s) are going to be used only here.
      VkShaderModule vertShaderModule;
      vertShaderModule = createShaderModule( vertShaderCode );

      // Actually creating the vertex shader here.
      VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
      vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = vertShaderModule;
      vertShaderStageInfo.pName = "main";

      VkPipelineShaderStageCreateInfo shaderStages[] =
        { vertShaderStageInfo };

      // Vertex input. We have one binding description and several attribute descriptions.
      VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      // BEGIN vk::Name (C++ style) to VkName conversion.
      VkVertexInputBindingDescription bindingDescription = lava::extras::Vertex::getBindingDescription( );
      auto cPlusPlusAttributeDescriptions = lava::extras::Vertex::getAttributeDescriptions( );
      // Only need the position here.
      std::array< VkVertexInputAttributeDescription, 1 > attributeDescriptions = {};
      attributeDescriptions[ 0 ] = cPlusPlusAttributeDescriptions[ 0 ];
      // END vk::Name (C++ style) to VkName conversion.
      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast< uint32_t >( attributeDescriptions.size( ) );
      vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
      vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data( );

      // Input assembly.
      VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
      inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      inputAssembly.primitiveRestartEnable = VK_FALSE;

      // Viewport.
      VkViewport viewport = {};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = ( float ) swapChainExtent.width;
      viewport.height = ( float ) swapChainExtent.height;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      // Scissors.
      VkRect2D scissor = {};
      scissor.offset = {0, 0};
      scissor.extent = swapChainExtent;

      // Viewport state = Viewport + Scissors.
      VkPipelineViewportStateCreateInfo viewportState = {};
      viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.viewportCount = 1;
      viewportState.pViewports = &viewport;
      viewportState.scissorCount = 1;
      viewportState.pScissors = &scissor;

      /** Rasterizer. **/
      VkPipelineRasterizationStateCreateInfo rasterizer = {};
      rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      // Frags out of frustum are only clamped (not discarded).
      rasterizer.depthClampEnable = VK_FALSE;
      // If true: no rasterizer action, no ouput.
      rasterizer.rasterizerDiscardEnable = VK_FALSE;
      // If fill is not used -> you must use a GPU feature.
      rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      // If it is greater than 1 -> you must use a GPU feature.
      rasterizer.lineWidth = 1.0f;
      rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
      // In Vulkan, Y is inverted.
      rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      // Some bias can be added to the depth values.
      rasterizer.depthBiasEnable = VK_FALSE;
      rasterizer.depthBiasConstantFactor = 0.0f; // Optional
      rasterizer.depthBiasClamp = 0.0f; // Optional
      rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

      /** Multisampling. **/
      // If you use it -> you must use a GPU feature.
      VkPipelineMultisampleStateCreateInfo multisampling = {};
      multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling.sampleShadingEnable = VK_FALSE;
      multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisampling.minSampleShading = 1.0f; // Optional
      multisampling.pSampleMask = nullptr; // Optional
      multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
      multisampling.alphaToOneEnable = VK_FALSE; // Optional

      // Depth and stencil testing. Not needed now.
      /**/
      VkPipelineDepthStencilStateCreateInfo depthStencil = {};
      depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      depthStencil.depthTestEnable = VK_TRUE;
      depthStencil.depthWriteEnable = VK_TRUE;
      depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
      depthStencil.depthBoundsTestEnable = VK_FALSE;
      depthStencil.minDepthBounds = 0.0f; // Optional
      depthStencil.maxDepthBounds = 1.0f; // Optional
      depthStencil.stencilTestEnable = VK_FALSE;
      depthStencil.front = {}; // Optional
      depthStencil.back = {}; // Optional
      /**/

      /** Color blending. **/
      /**
      // VkPipelineColorBlendAttachmentState for each framebuffer.
      VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
      colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment.blendEnable = VK_FALSE;
      colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
      colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
      colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
      colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
      colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
      colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

      // VkPipelineColorBlendStateCreateInfo for global config.
      VkPipelineColorBlendStateCreateInfo colorBlending = {};
      colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlending.logicOpEnable = VK_FALSE;
      colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
      colorBlending.attachmentCount = 1;
      colorBlending.pAttachments = &colorBlendAttachment;
      colorBlending.blendConstants[0] = 0.0f; // Optional
      colorBlending.blendConstants[1] = 0.0f; // Optional
      colorBlending.blendConstants[2] = 0.0f; // Optional
      colorBlending.blendConstants[3] = 0.0f; // Optional
      **/

      /** Dynamic state. **/
      // Option 1. This is to change some things in drawing time.
      /**
      VkDynamicState dynamicStates[] = {
          VK_DYNAMIC_STATE_VIEWPORT,
          VK_DYNAMIC_STATE_LINE_WIDTH
      };

      VkPipelineDynamicStateCreateInfo dynamicState = {};
      dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      dynamicState.dynamicStateCount = 2;
      dynamicState.pDynamicStates = dynamicStates;
      **/
      // Option 2.
      // VkPipelineDynamicStateCreateInfo dynamicState = nullptr;

      /** Pipeline layout. **/
      VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
      pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount = 1;
      pipelineLayoutInfo.pSetLayouts = &descriptorSetLayoutForFirstPass;
      pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangesForFirstPass.size( );
      pipelineLayoutInfo.pPushConstantRanges = pushConstantRangesForFirstPass.data( );

      if( vkCreatePipelineLayout( device, &pipelineLayoutInfo, nullptr, &pipelineLayoutForFirstPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create pipeline layout!" );
      }

      VkGraphicsPipelineCreateInfo pipelineInfo = {};
      pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipelineInfo.stageCount = 1;
      pipelineInfo.pStages = shaderStages;
      pipelineInfo.pVertexInputState = &vertexInputInfo;
      pipelineInfo.pInputAssemblyState = &inputAssembly;
      pipelineInfo.pViewportState = &viewportState;
      pipelineInfo.pRasterizationState = &rasterizer;
      pipelineInfo.pMultisampleState = &multisampling;
      pipelineInfo.pDepthStencilState = &depthStencil;
      pipelineInfo.pColorBlendState = nullptr; //&colorBlending;
      pipelineInfo.pDynamicState = nullptr; // Optional
      pipelineInfo.layout = pipelineLayoutForFirstPass;
      pipelineInfo.renderPass = firstRenderPass;
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
      pipelineInfo.basePipelineIndex = -1; // Optional

      if( vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineForFirstPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create graphics pipeline!" );
      }

      // Destroying VkShaderModule(s).
      vkDestroyShaderModule( device, vertShaderModule, nullptr );
    }

    void createGraphicsPipelineForSecondPass( void )
    {
      // Getting shaders byte array.
      auto vertShaderCode = readFile( shadowMappingSecondPassByJGMVertexShader );
      auto fragShaderCode = readFile( shadowMappingSecondPassByJGMFragmentShader );

      // Here we should check that byte size is equal to file byte size.

      // VkShaderModule(s) are going to be used only here.
      VkShaderModule vertShaderModule;
      VkShaderModule fragShaderModule;

      vertShaderModule = createShaderModule( vertShaderCode );
      fragShaderModule = createShaderModule( fragShaderCode );

      // Actually creating the vertex shader here.
      VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
      vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = vertShaderModule;
      vertShaderStageInfo.pName = "main";

      // Actually creating the fragment shader here.
      VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
      fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragShaderStageInfo.module = fragShaderModule;
      fragShaderStageInfo.pName = "main";

      VkPipelineShaderStageCreateInfo shaderStages[] =
        {vertShaderStageInfo, fragShaderStageInfo};

      // Vertex input. We have one binding description and several attribute descriptions.
      VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      // BEGIN vk::Name (C++ style) to VkName conversion.
      VkVertexInputBindingDescription bindingDescription = lava::extras::Vertex::getBindingDescription( );
      auto cPlusPlusAttributeDescriptions = lava::extras::Vertex::getAttributeDescriptions( );
      std::array< VkVertexInputAttributeDescription, 2 > attributeDescriptions = {};
      attributeDescriptions[ 0 ] = cPlusPlusAttributeDescriptions[ 0 ];
      attributeDescriptions[ 1 ] = cPlusPlusAttributeDescriptions[ 1 ];
      // END vk::Name (C++ style) to VkName conversion.
      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast< uint32_t >( attributeDescriptions.size( ) );
      vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
      vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data( );

      // Input assembly.
      VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
      inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      inputAssembly.primitiveRestartEnable = VK_FALSE;

      // Viewport.
      VkViewport viewport = {};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = ( float ) swapChainExtent.width;
      viewport.height = ( float ) swapChainExtent.height;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      // Scissors.
      VkRect2D scissor = {};
      scissor.offset = {0, 0};
      scissor.extent = swapChainExtent;

      // Viewport state = Viewport + Scissors.
      VkPipelineViewportStateCreateInfo viewportState = {};
      viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.viewportCount = 1;
      viewportState.pViewports = &viewport;
      viewportState.scissorCount = 1;
      viewportState.pScissors = &scissor;

      /** Rasterizer. **/
      VkPipelineRasterizationStateCreateInfo rasterizer = {};
      rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      // Frags out of frustum are only clamped (not discarded).
      rasterizer.depthClampEnable = VK_FALSE;
      // If true: no rasterizer action, no ouput.
      rasterizer.rasterizerDiscardEnable = VK_FALSE;
      // If fill is not used -> you must use a GPU feature.
      rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      // If it is greater than 1 -> you must use a GPU feature.
      rasterizer.lineWidth = 1.0f;
      rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
      // In Vulkan, Y is inverted.
      rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      // Some bias can be added to the depth values.
      rasterizer.depthBiasEnable = VK_FALSE;
      rasterizer.depthBiasConstantFactor = 0.0f; // Optional
      rasterizer.depthBiasClamp = 0.0f; // Optional
      rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

      /** Multisampling. **/
      // If you use it -> you must use a GPU feature.
      VkPipelineMultisampleStateCreateInfo multisampling = {};
      multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling.sampleShadingEnable = VK_FALSE;
      multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisampling.minSampleShading = 1.0f; // Optional
      multisampling.pSampleMask = nullptr; // Optional
      multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
      multisampling.alphaToOneEnable = VK_FALSE; // Optional

      // Depth and stencil testing. Not needed now.
      VkPipelineDepthStencilStateCreateInfo depthStencil = {};
      depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      depthStencil.depthTestEnable = VK_TRUE;
      depthStencil.depthWriteEnable = VK_TRUE;
      depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
      depthStencil.depthBoundsTestEnable = VK_FALSE;
      depthStencil.minDepthBounds = 0.0f; // Optional
      depthStencil.maxDepthBounds = 1.0f; // Optional
      depthStencil.stencilTestEnable = VK_FALSE;
      depthStencil.front = {}; // Optional
      depthStencil.back = {}; // Optional

      /** Color blending. **/
      // VkPipelineColorBlendAttachmentState for each framebuffer.
      VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
      colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment.blendEnable = VK_FALSE;
      colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
      colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
      colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
      colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
      colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
      colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

      // VkPipelineColorBlendStateCreateInfo for global config.
      VkPipelineColorBlendStateCreateInfo colorBlending = {};
      colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlending.logicOpEnable = VK_FALSE;
      colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
      colorBlending.attachmentCount = 1;
      colorBlending.pAttachments = &colorBlendAttachment;
      colorBlending.blendConstants[0] = 0.0f; // Optional
      colorBlending.blendConstants[1] = 0.0f; // Optional
      colorBlending.blendConstants[2] = 0.0f; // Optional
      colorBlending.blendConstants[3] = 0.0f; // Optional

      /** Dynamic state. **/
      // Option 1. This is to change some things in drawing time.
      /**
      VkDynamicState dynamicStates[] = {
          VK_DYNAMIC_STATE_VIEWPORT,
          VK_DYNAMIC_STATE_LINE_WIDTH
      };

      VkPipelineDynamicStateCreateInfo dynamicState = {};
      dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      dynamicState.dynamicStateCount = 2;
      dynamicState.pDynamicStates = dynamicStates;
      **/
      // Option 2.
      // VkPipelineDynamicStateCreateInfo dynamicState = nullptr;

      /** Pipeline layout. **/
      VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
      pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount = 1;
      pipelineLayoutInfo.pSetLayouts = &descriptorSetLayoutForSecondPass;
      pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangesForSecondPass.size( );
      pipelineLayoutInfo.pPushConstantRanges = pushConstantRangesForSecondPass.data( );

      if( vkCreatePipelineLayout( device, &pipelineLayoutInfo, nullptr, &pipelineLayoutForSecondPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create pipeline layout!" );
      }

      VkGraphicsPipelineCreateInfo pipelineInfo = {};
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
      pipelineInfo.pDynamicState = nullptr; // Optional
      pipelineInfo.layout = pipelineLayoutForSecondPass;
      pipelineInfo.renderPass = secondRenderPass;
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
      pipelineInfo.basePipelineIndex = -1; // Optional

      if( vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineForSecondPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create graphics pipeline!" );
      }

      // Destroying VkShaderModule(s).
      vkDestroyShaderModule( device, fragShaderModule, nullptr );
      vkDestroyShaderModule( device, vertShaderModule, nullptr );
    }

    void createFramebuffersForFirstPass( void )
    {
      swapChainFramebuffersForFirstPass.resize( swapChainImageViews.size( ) );

      // Creating a framebuffer foreach set of image views.
      for( size_t i = 0; i < swapChainImageViews.size( ); i++ )
      {
        std::array< VkImageView, 1 > attachments =
        {
          shadowMapImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = firstRenderPass;
        framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size( ) );
        framebufferInfo.pAttachments = attachments.data( );
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if( vkCreateFramebuffer( device, &framebufferInfo, nullptr, &swapChainFramebuffersForFirstPass[i] ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to create framebuffer!" );
        }
      }
    }

    void createFramebuffersForSecondPass( void )
    {
      swapChainFramebuffersForSecondPass.resize( swapChainImageViews.size( ) );

      // Creating a framebuffer foreach set of image views.
      for( size_t i = 0; i < swapChainImageViews.size( ); i++ )
      {
        std::array< VkImageView, 2 > attachments =
        {
          swapChainImageViews[i],
          depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = secondRenderPass;
        framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size( ) );
        framebufferInfo.pAttachments = attachments.data( );
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if( vkCreateFramebuffer( device, &framebufferInfo, nullptr, &swapChainFramebuffersForSecondPass[i] ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to create framebuffer!" );
        }
      }
    }

    void createCommandPool( void )
    {
      QueueFamilyIndices queueFamilyIndices = findQueueFamilies( physicalDevice );

      VkCommandPoolCreateInfo poolInfo = {};
      poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
      poolInfo.flags = 0; // Optional

      if( vkCreateCommandPool( device, &poolInfo, nullptr, &commandPool ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create command pool!" );
      }
    }

    void createDepthResources( void )
    {
      VkFormat depthFormat = findDepthFormat( );
      createImage( swapChainExtent.width, swapChainExtent.height,
                   depthFormat, VK_IMAGE_TILING_OPTIMAL,
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                   depthImage, depthImageMemory);
      depthImageView = createImageView( depthImage, depthFormat,
                                        VK_IMAGE_ASPECT_DEPTH_BIT );
      transitionImageLayout( depthImage, depthFormat,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
    }

    VkFormat findSupportedFormat( const std::vector< VkFormat >& candidates,
                                  VkImageTiling tiling,
                                  VkFormatFeatureFlags features )
    {
      for( VkFormat format : candidates )
      {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties( physicalDevice, format, &props );

        if( tiling == VK_IMAGE_TILING_LINEAR &&
            ( props.linearTilingFeatures & features ) == features )
        {
          return format;
        }
        else if( tiling == VK_IMAGE_TILING_OPTIMAL &&
                 ( props.optimalTilingFeatures & features ) == features )
        {
          return format;
        }
      }

      throw std::runtime_error( "failed to find supported format!" );
    }

    VkFormat findDepthFormat( void )
    {
      return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT,
          VK_FORMAT_D32_SFLOAT_S8_UINT,
          VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
      );
    }

    VkFormat findDepthFormatForShadowMap( void )
    {
      return findSupportedFormat(
        { VK_FORMAT_D16_UNORM,
          VK_FORMAT_D32_SFLOAT,
          VK_FORMAT_D32_SFLOAT_S8_UINT,
          VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
      );
    }

    bool hasStencilComponent( VkFormat format )
    {
      return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
             format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void createShadowMapImageAndImageView( void )
    {
      VkFormat depthFormat = findDepthFormatForShadowMap( );
      createImage( swapChainExtent.width, swapChainExtent.height,
                   depthFormat, VK_IMAGE_TILING_OPTIMAL,
                   VK_IMAGE_USAGE_SAMPLED_BIT |
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                   shadowMapImage, shadowMapImageMemory);
      shadowMapImageView = createImageView( shadowMapImage, depthFormat,
                                            VK_IMAGE_ASPECT_DEPTH_BIT );
      /**/
      transitionImageLayout( shadowMapImage, depthFormat,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
      /**/
    }

    void createShadowMapSampler( void )
    {
      VkSamplerCreateInfo samplerInfo = {};
      samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      samplerInfo.magFilter = VK_FILTER_LINEAR;
      samplerInfo.minFilter = VK_FILTER_LINEAR;
      samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      // If anisotropy is disabled use 1 for maxAnisotropy.
      samplerInfo.anisotropyEnable = VK_FALSE;
      samplerInfo.maxAnisotropy = 1;
      samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      samplerInfo.unnormalizedCoordinates = VK_FALSE;
      samplerInfo.compareEnable = VK_FALSE;
      samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
      samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      samplerInfo.mipLodBias = 0.0f;
      samplerInfo.minLod = 0.0f;
      samplerInfo.maxLod = 100.0f;

      if( vkCreateSampler( device, &samplerInfo, nullptr, &shadowMapSampler ) != VK_SUCCESS )
      {
        throw std::runtime_error("failed to create texture sampler!");
      }
    }

    VkImageView createImageView( VkImage image, VkFormat format,
                                 VkImageAspectFlags aspectFlags )
    {
      VkImageViewCreateInfo viewInfo = {};
      viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewInfo.image = image;
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = format;
      viewInfo.subresourceRange.aspectMask = aspectFlags;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.layerCount = 1;

      VkImageView imageView;
      if( vkCreateImageView( device, &viewInfo, nullptr, &imageView ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create texture image view!" );
      }

      return imageView;
    }

    void createImage( uint32_t width, uint32_t height,
                      VkFormat format, VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkImage& image, VkDeviceMemory& imageMemory )
    {
      VkImageCreateInfo imageInfo = {};
      imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageInfo.imageType = VK_IMAGE_TYPE_2D;
      imageInfo.extent.width = width;
      imageInfo.extent.height = height;
      imageInfo.extent.depth = 1;
      imageInfo.mipLevels = 1;
      imageInfo.arrayLayers = 1;
      imageInfo.format = format;
      imageInfo.tiling = tiling;
      imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      imageInfo.usage = usage;
      imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if( vkCreateImage( device, &imageInfo, nullptr, &image ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create image!" );
      }

      VkMemoryRequirements memRequirements;
      vkGetImageMemoryRequirements( device, image, &memRequirements );

      VkMemoryAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize = memRequirements.size;
      allocInfo.memoryTypeIndex =
        findMemoryType( memRequirements.memoryTypeBits, properties );

      if( vkAllocateMemory( device, &allocInfo, nullptr, &imageMemory ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to allocate image memory!" );
      }

      vkBindImageMemory( device, image, imageMemory, 0 );
    }

    void transitionImageLayout( VkImage image, VkFormat format,
                                VkImageLayout oldLayout,
                                VkImageLayout newLayout )
    {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands( );

      VkImageMemoryBarrier barrier = {};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.oldLayout = oldLayout;
      barrier.newLayout = newLayout;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.image = image;
      if( newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
      {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if( hasStencilComponent( format ) )
        {
          barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
      }
      else
      {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      }
      barrier.subresourceRange.baseMipLevel = 0;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.baseArrayLayer = 0;
      barrier.subresourceRange.layerCount = 1;

      VkPipelineStageFlags sourceStage;
      VkPipelineStageFlags destinationStage;

      if( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
          newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
      {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      }
      else if( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
      {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      }
      else if( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
      {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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
    }

    void copyBufferToImage( VkBuffer buffer, VkImage image,
                            uint32_t width, uint32_t height )
    {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands( );

      VkBufferImageCopy region = {};
      region.bufferOffset = 0;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;
      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = 0;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = 1;
      region.imageOffset = { 0, 0, 0 };
      region.imageExtent = { width, height, 1 };

      vkCmdCopyBufferToImage( commandBuffer, buffer, image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
        &region );

      endSingleTimeCommands( commandBuffer );
    }

    void createVertexBuffers( void )
    {
      // 1: Bunny.

      VkDeviceSize bunnyBufferSize = sizeof( bunnyMesh.vertices[0] ) * bunnyMesh.numVertices;

      // Staging buffer.
      VkBuffer bunnyStagingBuffer;
      VkDeviceMemory bunnyStagingBufferMemory;
      createBuffer( bunnyBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    bunnyStagingBuffer,
                    bunnyStagingBufferMemory );

      // Data mapping.
      void* bunnyData;
      vkMapMemory( device, bunnyStagingBufferMemory, 0, bunnyBufferSize, 0, &bunnyData );
      memcpy( bunnyData, bunnyMesh.vertices.data( ), (size_t) bunnyBufferSize );
      vkUnmapMemory( device, bunnyStagingBufferMemory );

      // Actual vertex buffer.
      createBuffer( bunnyBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    bunnyVertexBuffer,
                    bunnyVertexBufferMemory );

      copyBuffer( bunnyStagingBuffer, bunnyVertexBuffer, bunnyBufferSize );

      vkDestroyBuffer( device, bunnyStagingBuffer, nullptr );
      vkFreeMemory( device, bunnyStagingBufferMemory, nullptr );

      // 2: Table.

      VkDeviceSize tableBufferSize = sizeof( tableMesh.vertices[0] ) * tableMesh.numVertices;

      // Staging buffer.
      VkBuffer tableStagingBuffer;
      VkDeviceMemory tableStagingBufferMemory;
      createBuffer( tableBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    tableStagingBuffer,
                    tableStagingBufferMemory );

      // Data mapping.
      void* tableData;
      vkMapMemory( device, tableStagingBufferMemory, 0, tableBufferSize, 0, &tableData );
      memcpy( tableData, tableMesh.vertices.data( ), (size_t) tableBufferSize );
      vkUnmapMemory( device, tableStagingBufferMemory );

      // Actual vertex buffer.
      createBuffer( tableBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    tableVertexBuffer,
                    tableVertexBufferMemory );

      copyBuffer( tableStagingBuffer, tableVertexBuffer, tableBufferSize );

      vkDestroyBuffer( device, tableStagingBuffer, nullptr );
      vkFreeMemory( device, tableStagingBufferMemory, nullptr );
    }

    void createIndexBuffers( void )
    {
      // 1: Bunny.

      VkDeviceSize bunnyBufferSize = sizeof( bunnyMesh.indices[0] ) * bunnyMesh.numIndices;

      // Staging buffer.
      VkBuffer bunnyStagingBuffer;
      VkDeviceMemory bunnyStagingBufferMemory;
      createBuffer( bunnyBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    bunnyStagingBuffer,
                    bunnyStagingBufferMemory );

      // Data mapping.
      void* bunnyData;
      vkMapMemory( device, bunnyStagingBufferMemory, 0, bunnyBufferSize, 0, &bunnyData );
      memcpy( bunnyData, bunnyMesh.indices.data( ), (size_t) bunnyBufferSize );
      vkUnmapMemory( device, bunnyStagingBufferMemory );

      // Actual index buffer.
      createBuffer( bunnyBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    bunnyIndexBuffer,
                    bunnyIndexBufferMemory );

      copyBuffer( bunnyStagingBuffer, bunnyIndexBuffer, bunnyBufferSize );

      vkDestroyBuffer( device, bunnyStagingBuffer, nullptr );
      vkFreeMemory( device, bunnyStagingBufferMemory, nullptr );

      // 2: Table.

      VkDeviceSize tableBufferSize = sizeof( tableMesh.indices[0] ) * tableMesh.numIndices;

      // Staging buffer.
      VkBuffer tableStagingBuffer;
      VkDeviceMemory tableStagingBufferMemory;
      createBuffer( tableBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    tableStagingBuffer,
                    tableStagingBufferMemory );

      // Data mapping.
      void* tableData;
      vkMapMemory( device, tableStagingBufferMemory, 0, tableBufferSize, 0, &tableData );
      memcpy( tableData, tableMesh.indices.data( ), (size_t) tableBufferSize );
      vkUnmapMemory( device, tableStagingBufferMemory );

      // Actual index buffer.
      createBuffer( tableBufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    tableIndexBuffer,
                    tableIndexBufferMemory );

      copyBuffer( tableStagingBuffer, tableIndexBuffer, tableBufferSize );

      vkDestroyBuffer( device, tableStagingBuffer, nullptr );
      vkFreeMemory( device, tableStagingBufferMemory, nullptr );
    }

    void createUniformBuffer( void )
    {
      VkDeviceSize bufferSize = sizeof( UniformBufferObject );
      createBuffer( bufferSize,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    uniformBuffer, uniformBufferMemory );
    }

    void initUniformBuffer( void )
    {
      UniformBufferObject defaultUniformBuffer = {};
      defaultUniformBuffer.cameraView =
        glm::lookAt( defaultCameraPosition,
                     glm::vec3( 0.0f, 0.0f, 0.0f ),
                     glm::vec3( -10.0f, 0.0f, 1.0f ) );
      defaultUniformBuffer.lightView =
        glm::lookAt( defaultLightPosition,
                     glm::vec3( 0.0f, 0.0f, -3.0f ),
                     glm::vec3( 0.0f, 0.0f, 1.0f ) );
      defaultUniformBuffer.proj =
        glm::perspective( glm::radians(45.0f),
                          swapChainExtent.width /
                          (float) swapChainExtent.height,
                          0.1f, 100.0f );
      // GLM was written for OpenGL. Vulkan has the Y inverted.
      defaultUniformBuffer.proj[1][1] *= -1;

      void* data;
      vkMapMemory( device, uniformBufferMemory, 0, sizeof( defaultUniformBuffer ), 0, &data );
      memcpy( data, &defaultUniformBuffer, sizeof( defaultUniformBuffer ) );
      vkUnmapMemory( device, uniformBufferMemory );
    }

    void createDescriptorPoolForFirstPass( void )
    {
      std::array< VkDescriptorPoolSize, 1 > poolSizes = {};
      poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSizes[0].descriptorCount = 1;

      VkDescriptorPoolCreateInfo poolInfo = {};
      poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size( ) );
      poolInfo.pPoolSizes = poolSizes.data( );
      poolInfo.maxSets = 1;

      if( vkCreateDescriptorPool( device, &poolInfo, nullptr, &descriptorPoolForFirstPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create descriptor pool!" );
      }
    }

    void createDescriptorPoolForSecondPass( void )
    {
      std::array< VkDescriptorPoolSize, 2 > poolSizes = {};
      poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSizes[0].descriptorCount = 1;
      poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      poolSizes[1].descriptorCount = 1;

      VkDescriptorPoolCreateInfo poolInfo = {};
      poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size( ) );
      poolInfo.pPoolSizes = poolSizes.data( );
      poolInfo.maxSets = 1;

      if( vkCreateDescriptorPool( device, &poolInfo, nullptr, &descriptorPoolForSecondPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create descriptor pool!" );
      }
    }

    void createDescriptorSetForFirstPass( void )
    {
      VkDescriptorSetLayout layouts[] = { descriptorSetLayoutForFirstPass };
      VkDescriptorSetAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = descriptorPoolForFirstPass;
      allocInfo.descriptorSetCount = 1;
      allocInfo.pSetLayouts = layouts;

      if( vkAllocateDescriptorSets( device, &allocInfo, &descriptorSetForFirstPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to allocate descriptor set!" );
      }
    }

    void updateDescriptorSetForFirstPass( void )
    {
      // Uniform buffer.
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = uniformBuffer;
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof( UniformBufferObject );

      std::array< VkWriteDescriptorSet, 1 > descriptorWrites = {};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = descriptorSetForFirstPass;
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      vkUpdateDescriptorSets( device, static_cast< uint32_t >( descriptorWrites.size( ) ),
                              descriptorWrites.data( ), 0, nullptr );
    }

    void createDescriptorSetForSecondPass( void )
    {
      VkDescriptorSetLayout layouts[] = { descriptorSetLayoutForSecondPass };
      VkDescriptorSetAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = descriptorPoolForSecondPass;
      allocInfo.descriptorSetCount = 1;
      allocInfo.pSetLayouts = layouts;

      if( vkAllocateDescriptorSets( device, &allocInfo, &descriptorSetForSecondPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to allocate descriptor set!" );
      }
    }

    void updateDescriptorSetForSecondPass( void )
    {
      // Uniform buffer.
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = uniformBuffer;
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof( UniformBufferObject );

      // Shadow map.
      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
      imageInfo.imageView = shadowMapImageView;
      imageInfo.sampler = shadowMapSampler;

      std::array< VkWriteDescriptorSet, 2 > descriptorWrites = {};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = descriptorSetForSecondPass;
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = descriptorSetForSecondPass;
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pImageInfo = &imageInfo;

      vkUpdateDescriptorSets( device, static_cast< uint32_t >( descriptorWrites.size( ) ),
                              descriptorWrites.data( ), 0, nullptr );
    }

    void createBuffer( VkDeviceSize size, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags properties, VkBuffer& buffer,
                       VkDeviceMemory& bufferMemory )
    {
      VkBufferCreateInfo bufferInfo = {};
      bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferInfo.size = size;
      bufferInfo.usage = usage;
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      // bufferInfo.flags = 0; // sparse buffer memory if not 0.

      if( vkCreateBuffer( device, &bufferInfo, nullptr, &buffer ) != VK_SUCCESS )
      {
        throw std::runtime_error("failed to create vertex buffer!");
      }

      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements( device, buffer, &memRequirements );

      VkMemoryAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize = memRequirements.size;
      allocInfo.memoryTypeIndex =
        findMemoryType( memRequirements.memoryTypeBits,
                        properties );

      if( vkAllocateMemory( device, &allocInfo, nullptr, &bufferMemory ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to allocate vertex buffer memory!" );
      }

      vkBindBufferMemory( device, buffer, bufferMemory, 0 );
    }

    VkCommandBuffer beginSingleTimeCommands( void )
    {
      VkCommandBufferAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool = commandPool;
      allocInfo.commandBufferCount = 1;

      VkCommandBuffer commandBuffer;
      vkAllocateCommandBuffers( device, &allocInfo, &commandBuffer );

      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      vkBeginCommandBuffer( commandBuffer, &beginInfo );

      return commandBuffer;
    }

    void endSingleTimeCommands( VkCommandBuffer commandBuffer )
    {
      vkEndCommandBuffer( commandBuffer );

      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffer;

      vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
      vkQueueWaitIdle( graphicsQueue );

      vkFreeCommandBuffers( device, commandPool, 1, &commandBuffer );
    }

    void copyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size )
    {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands( );

      VkBufferCopy copyRegion = {};
      copyRegion.srcOffset = 0; // Optional
      copyRegion.dstOffset = 0; // Optional
      copyRegion.size = size;
      vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

      endSingleTimeCommands( commandBuffer );
    }

    uint32_t findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties )
    {
      VkPhysicalDeviceMemoryProperties memProperties;
      vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );

      for( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
      {
        if( ( typeFilter & ( 1 << i ) ) &&
            ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties )
        {
          return i;
        }
      }

      throw std::runtime_error( "failed to find suitable memory type!" );
    }

    void createCommandBuffers( void )
    {
      commandBuffers.resize( swapChainImageViews.size( ) );

      VkCommandBufferAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = commandPool;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = ( uint32_t ) commandBuffers.size( );

      if( vkAllocateCommandBuffers( device, &allocInfo, commandBuffers.data( ) ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to allocate command buffers!" );
      }

      // VertexBuffers.
      VkBuffer bunnyVertexBuffers[] = { bunnyVertexBuffer };
      VkDeviceSize bunnyVertexBufferOffsets[] = { 0 };

      VkBuffer tableVertexBuffers[] = { tableVertexBuffer };
      VkDeviceSize tableVertexBufferOffsets[] = { 0 };

      // PushConstants.
      glm::mat4 bunnyModelMatrix = glm::mat4( 1.0f );
      bunnyModelMatrix[ 3 ] = glm::vec4( 0.0f, 5.0f, 0.0f, 1.0f );

      glm::mat4 tableModelMatrix = glm::mat4( 1.0f );
      tableModelMatrix[ 0 ].x = 20.0f;
      tableModelMatrix[ 1 ].y = 1.0f;
      tableModelMatrix[ 2 ].z = 20.0f;

      glm::vec4 initialLightPosition = glm::vec4( defaultLightPosition, 1.0f );

      for( size_t i = 0; i < commandBuffers.size( ); i++ )
      {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        vkBeginCommandBuffer( commandBuffers[i], &beginInfo );

        VkRenderPassBeginInfo firstRenderPassInfo = {};
        firstRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        firstRenderPassInfo.renderPass = firstRenderPass;
        firstRenderPassInfo.framebuffer = swapChainFramebuffersForFirstPass[i];
        firstRenderPassInfo.renderArea.offset = {0, 0};
        firstRenderPassInfo.renderArea.extent = swapChainExtent;
        std::array< VkClearValue, 1 > clearValuesForFirstPass = {};
        clearValuesForFirstPass[0].depthStencil = {1.0f, 0};
        firstRenderPassInfo.clearValueCount = static_cast< uint32_t >( clearValuesForFirstPass.size( ) );
        firstRenderPassInfo.pClearValues = clearValuesForFirstPass.data( );

        // BEGIN First render pass.
        vkCmdBeginRenderPass( commandBuffers[i], &firstRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

          vkCmdBindPipeline( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineForFirstPass );

          // 1: Bunny.

          // Working with buffers.
          vkCmdBindVertexBuffers( commandBuffers[i], 0, 1,
                                  bunnyVertexBuffers, bunnyVertexBufferOffsets );

          // To read meshes it is better to use VK_INDEX_TYPE_UINT32 instead of VK_INDEX_TYPE_UINT16.
          vkCmdBindIndexBuffer( commandBuffers[i], bunnyIndexBuffer, 0, VK_INDEX_TYPE_UINT32 );

          // Descriptor sets: a uniform buffer (view and proj).
          vkCmdBindDescriptorSets( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   pipelineLayoutForFirstPass, 0, 1, &descriptorSetForFirstPass, 0, nullptr );

          vkCmdPushConstants( commandBuffers[i], pipelineLayoutForFirstPass,
                              VK_SHADER_STAGE_VERTEX_BIT,
                              0, 16 * sizeof( float ),
                              &bunnyModelMatrix );

          vkCmdDrawIndexed(
            commandBuffers[i],
            static_cast< uint32_t >( bunnyMesh.numIndices ),
            1, 0, 0, 0 );

          // 2: Table.

          vkCmdBindVertexBuffers( commandBuffers[i], 0, 1,
                                  tableVertexBuffers, tableVertexBufferOffsets );

          vkCmdBindIndexBuffer( commandBuffers[i], tableIndexBuffer, 0, VK_INDEX_TYPE_UINT32 );

          vkCmdPushConstants( commandBuffers[i], pipelineLayoutForFirstPass,
                              VK_SHADER_STAGE_VERTEX_BIT,
                              0, 16 * sizeof( float ),
                              &tableModelMatrix );

          vkCmdDrawIndexed(
            commandBuffers[i],
            static_cast< uint32_t >( tableMesh.numIndices ),
            1, 0, 0, 0 );

        vkCmdEndRenderPass( commandBuffers[i] );
        // END First render pass.

        VkRenderPassBeginInfo secondRenderPassInfo = {};
        secondRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        secondRenderPassInfo.renderPass = secondRenderPass;
        secondRenderPassInfo.framebuffer = swapChainFramebuffersForSecondPass[i];
        secondRenderPassInfo.renderArea.offset = {0, 0};
        secondRenderPassInfo.renderArea.extent = swapChainExtent;
        std::array< VkClearValue, 2 > clearValuesForSecondPass = {};
        clearValuesForSecondPass[0].color = {0.3f, 0.65f, 1.0f, 1.0f};
        clearValuesForSecondPass[1].depthStencil = {1.0f, 0};
        secondRenderPassInfo.clearValueCount = static_cast< uint32_t >( clearValuesForSecondPass.size( ) );
        secondRenderPassInfo.pClearValues = clearValuesForSecondPass.data( );

        // BEGIN Second render pass.
        vkCmdBeginRenderPass( commandBuffers[i], &secondRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

          vkCmdBindPipeline( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineForSecondPass );

          // 1: Bunny.

          // Working with buffers.
          vkCmdBindVertexBuffers( commandBuffers[i], 0, 1,
                                  bunnyVertexBuffers, bunnyVertexBufferOffsets );

          // To read meshes it is better to use VK_INDEX_TYPE_UINT32 instead of VK_INDEX_TYPE_UINT16.
          vkCmdBindIndexBuffer( commandBuffers[i], bunnyIndexBuffer, 0, VK_INDEX_TYPE_UINT32 );

          // Descriptor sets: a uniform buffer (view and proj).
          vkCmdBindDescriptorSets( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   pipelineLayoutForSecondPass, 0, 1, &descriptorSetForSecondPass, 0, nullptr );

          // Push constants: a model matrix and a view-space light position.
          vkCmdPushConstants( commandBuffers[i], pipelineLayoutForSecondPass,
                              VK_SHADER_STAGE_VERTEX_BIT,
                              0, 16 * sizeof( float ),
                              &bunnyModelMatrix );
          vkCmdPushConstants( commandBuffers[i], pipelineLayoutForSecondPass,
                              VK_SHADER_STAGE_VERTEX_BIT,
                              16 * sizeof( float ), 4 * sizeof( float ),
                              &initialLightPosition );

          vkCmdDrawIndexed(
            commandBuffers[i],
            static_cast< uint32_t >( bunnyMesh.numIndices ),
            1, 0, 0, 0 );

          // 2: Table.
          vkCmdBindVertexBuffers( commandBuffers[i], 0, 1,
                                  tableVertexBuffers, tableVertexBufferOffsets );

          vkCmdBindIndexBuffer( commandBuffers[i], tableIndexBuffer, 0, VK_INDEX_TYPE_UINT32 );

          vkCmdPushConstants( commandBuffers[i], pipelineLayoutForSecondPass,
                              VK_SHADER_STAGE_VERTEX_BIT,
                              0, 16 * sizeof( float ),
                              &tableModelMatrix );

          vkCmdDrawIndexed(
            commandBuffers[i],
            static_cast< uint32_t >( tableMesh.numIndices ),
            1, 0, 0, 0 );

        vkCmdEndRenderPass( commandBuffers[i] );
        // END Second render pass.

        if( vkEndCommandBuffer( commandBuffers[i] ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to record command buffer!" );
        }
      }
    }

    void createSemaphores( void )
    {
      VkSemaphoreCreateInfo semaphoreInfo = {};
      semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      if( vkCreateSemaphore( device, &semaphoreInfo, nullptr, &imageAvailableSemaphore ) != VK_SUCCESS ||
          vkCreateSemaphore( device, &semaphoreInfo, nullptr, &renderFinishedSemaphore ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create semaphores!" );
      }
    }

    /**
    void updateUniformBuffer( void )
    {
      static auto startTime = std::chrono::high_resolution_clock::now( );

      auto currentTime = std::chrono::high_resolution_clock::now( );
      float time = std::chrono::duration< float, std::chrono::seconds::period >
                   ( currentTime - startTime ).count( );

      UniformBufferObject ubo = {};
      ubo.model = glm::rotate( glm::mat4( 1.0f ),
                               time * glm::radians( 45.0f ),
                               glm::vec3( 0.0f, 1.0f, 0.0f ) );
      ubo.view = glm::lookAt( glm::vec3( 0.0f, 0.01f, 7.0f ),
                              glm::vec3( 0.0f, 0.0f, 0.0f ),
                              glm::vec3( 0.0f, 0.0f, -1.0f ) );
      ubo.proj = glm::perspective( glm::radians(45.0f),
                                   swapChainExtent.width /
                                   (float) swapChainExtent.height,
                                   0.1f, 100.0f );
      // GLM was written for OpenGL. Vulkan has the Y inverted.
      ubo.proj[1][1] *= -1;

      void* data;
      vkMapMemory( device, uniformBufferMemory, 0, sizeof( ubo ), 0, &data );
      memcpy( data, &ubo, sizeof( ubo ) );
      vkUnmapMemory( device, uniformBufferMemory );
    }
    **/

    void drawFrame( void )
    {
      // Acquiring an image from the swap chain.
      uint32_t imageIndex;
      VkResult result = vkAcquireNextImageKHR(
                          device, swapChain,
                          std::numeric_limits< uint64_t >::max( ), imageAvailableSemaphore,
                          VK_NULL_HANDLE, &imageIndex );

      // Swapchain could be out of date or surface could be suboptimal for the Sc.
      if( result == VK_ERROR_OUT_OF_DATE_KHR )
      {
        recreateSwapChain( );
        return;
      }
      else if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
      {
        throw std::runtime_error( "failed to acquire swap chain image!" );
      }

      // Submitting the command buffer.
      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

      VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = waitSemaphores;
      submitInfo.pWaitDstStageMask = waitStages;

      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

      VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = signalSemaphores;

      if( vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to submit draw command buffer!" );
      }

      // Presentation.
      VkPresentInfoKHR presentInfo = {};
      presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

      presentInfo.waitSemaphoreCount = 1;
      presentInfo.pWaitSemaphores = signalSemaphores;

      VkSwapchainKHR swapChains[] = { swapChain };
      presentInfo.swapchainCount = 1;
      presentInfo.pSwapchains = swapChains;
      presentInfo.pImageIndices = &imageIndex;
      presentInfo.pResults = nullptr; // Optional

      result = vkQueuePresentKHR( presentQueue, &presentInfo );

      // In this case we will be more strict.
      if( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
      {
        recreateSwapChain( );
      }
      else if( result != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to present swap chain image!" );
      }

      // Optional: explicitly waiting for presentation to finish.
      vkQueueWaitIdle( presentQueue );
    }

    /** Auxiliary functions. **/

    VkShaderModule createShaderModule( const std::vector< char >& code )
    {
      VkShaderModuleCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      createInfo.codeSize = code.size( );
      createInfo.pCode = reinterpret_cast< const uint32_t* >( code.data( ) );

      VkShaderModule shaderModule;
      if( vkCreateShaderModule( device, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create shader module!" );
      }

      return shaderModule;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableFormats )
    {
      // VkSurfaceFormat formed by format and colorSpace.
      if( availableFormats.size( ) == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED )
      {
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
      }

      // availableFormats.size( ) > 1.
      for( const auto& availableFormat : availableFormats )
      {
        if( availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
        {
          return availableFormat;
        }
      }

      /** Option 1. **/
      // Make a ranking and choose the best foramt.

      /** Option 2. **/
      return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode( const std::vector<VkPresentModeKHR> availablePresentModes )
    {
      // MAILBOX > IMMEDIATE > FIFO.
      VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

      for( const auto& availablePresentMode : availablePresentModes )
      {
        if( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
        {
          return availablePresentMode;
        }
        else if( availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
        {
          bestMode = availablePresentMode;
        }
      }

      return bestMode;
    }

    VkExtent2D chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities )
    {
      if( capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max( ) )
      {
        return capabilities.currentExtent;
      }
      else
      {
        /** Option 1. Constants. **/
        // VkExtent2D actualExtent = {WIDTH, HEIGHT};

        /** Option 2. Variables. **/
        int width;
        int height;
        glfwGetWindowSize( window, &width, &height );
        VkExtent2D actualExtent = {width, height};

        actualExtent.width = std::max( capabilities.minImageExtent.width,
                                       std::min( capabilities.maxImageExtent.width, actualExtent.width ) );
        actualExtent.height = std::max( capabilities.minImageExtent.height,
                                        std::min( capabilities.maxImageExtent.height, actualExtent.height ) );

        return actualExtent;
      }
    }

    SwapChainSupportDetails querySwapChainSupport( VkPhysicalDevice device )
    {
      SwapChainSupportDetails details;

      // SwapChainSupportDetails.capabilities
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &details.capabilities );

      // SwapChainSupportDetails.formats
      uint32_t formatCount;
      vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &formatCount, nullptr );

      if( formatCount != 0 )
      {
        details.formats.resize( formatCount );
        vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &formatCount, details.formats.data( ) );
      }

      // SwapChainSupportDetails.presentModes
      uint32_t presentModeCount;
      vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, nullptr );

      if( presentModeCount != 0 )
      {
        details.presentModes.resize( presentModeCount );
        vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, details.presentModes.data( ) );
      }

      return details;
    }

    bool isDeviceSuitable( VkPhysicalDevice device )
    {
      /** Option 1. **/
      /**
      // Properties (basic).
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties( device, &deviceProperties );

      // Features (optional).
      VkPhysicalDeviceFeatures deviceFeatures;
      vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

      // Example: we require the use of geometry shaders.
      return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
             deviceFeatures.geometryShader;
      **/

      /** Option 2. **/
      // Give each device a score and chose the highest one.

      /** Option 3. **/
      // User choice.

      /** Option 4. **/
      // Any GPU.
      // return true;

      /** Option 5. **/
      // Conditions.

      // 1: Queues with VK_QUEUE_GRAPHICS_BIT and SurfaceSupportKHR.
      QueueFamilyIndices indices = findQueueFamilies( device );

      // 2: Device with VK_KHR_swapchain extension.
      bool extensionsSupported = checkDeviceExtensionSupport( device );

      // 3: SwapChainSupportDetails are sufficiently good.
      // At least one format and one present mode.
      bool swapChainAdequate = false;
      if( extensionsSupported )
      {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport( device );
        swapChainAdequate = !swapChainSupport.formats.empty( ) && !swapChainSupport.presentModes.empty( );
      }

      // 4: Some physical device features.
      VkPhysicalDeviceFeatures supportedFeatures;
      vkGetPhysicalDeviceFeatures( device, &supportedFeatures );

      return indices.isComplete( ) &&
             extensionsSupported &&
             swapChainAdequate &&
             supportedFeatures.samplerAnisotropy;
    }

    bool checkDeviceExtensionSupport( VkPhysicalDevice device )
    {
      uint32_t extensionCount;
      vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

      std::vector< VkExtensionProperties > availableExtensions( extensionCount );
      vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data( ) );

      std::set< std::string > requiredExtensions( deviceExtensions.begin( ), deviceExtensions.end( ) );

      // Nice trick to know if all required extensions have been found.
      for( const auto& extension : availableExtensions )
      {
        requiredExtensions.erase( extension.extensionName );
      }

      return requiredExtensions.empty( );
    }

    QueueFamilyIndices findQueueFamilies( VkPhysicalDevice device )
    {
      QueueFamilyIndices indices;

      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );

      std::vector< VkQueueFamilyProperties > queueFamilies( queueFamilyCount );
      vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data( ) );

      int i = 0;
      for( const auto& queueFamily : queueFamilies )
      {
        /** graphicsFamily checking. **/
        if( queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
          indices.graphicsFamily = i;
        }

        /** presentFamily checking. **/
        // Surface support KHR.
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &presentSupport );

        if( queueFamily.queueCount > 0 && presentSupport )
        {
          indices.presentFamily = i;
        }

        if( indices.isComplete( ) )
        {
          break;
        }

        i++;
      }

      return indices;
    }

    std::vector< const char* > getRequiredExtensions( void )
    {
      std::vector< const char* > extensions;

      unsigned int glfwExtensionCount = 0;
      const char** glfwExtensions;
      glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

      for( unsigned int i = 0; i < glfwExtensionCount; i++ )
      {
        extensions.push_back( glfwExtensions[i] );
      }

      // Additional extension: VK_EXT_debug_report.
      if( enableValidationLayers )
      {
        extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
      }

      return extensions;
    }

    bool checkValidationLayerSupport( void )
    {
      uint32_t layerCount;
      vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

      std::vector< VkLayerProperties > availableLayers( layerCount );
      vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data( ) );

      // Check if all of the layers in validationLayers exist in the availableLayers list.
      for( const char* layerName : validationLayers )
      {
        bool layerFound = false;

        for( const auto& layerProperties : availableLayers )
        {
          if( strcmp( layerName, layerProperties.layerName ) == 0 )
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

    static std::vector< char > readFile( const std::string& filename )
    {
      std::ifstream file( filename, std::ios::ate | std::ios::binary );

      if( !file.is_open( ) )
      {
        throw std::runtime_error( "failed to open file!" );
      }

      // Getting file size.
      size_t fileSize = (size_t) file.tellg( );
      std::vector< char > buffer( fileSize );

      // Going back to the start and read.
      file.seekg( 0 );
      file.read( buffer.data( ), fileSize );

      // Closing and returning the bytes.
      file.close( );

      return buffer;
    }

    // Following the PFN_vkDebugReportCallbackEXT prototype.
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugReportFlagsEXT flags,
      VkDebugReportObjectTypeEXT objType,
      uint64_t obj,
      size_t location,
      int32_t code,
      const char* layerPrefix,
      const char* msg,
      void* userData)
    {
      std::cerr << "validation layer: " << msg << std::endl;

      return VK_FALSE;
    }

};

int main( )
{
  HelloTriangleApplication app;

  try
  {
    app.run();
  }
  catch (const std::runtime_error& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
