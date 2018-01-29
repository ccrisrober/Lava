/**
 * Lava.
 * File: UniformBuffersByJGM.cpp
 * Author: Juan Guerrero Martín.
 * Brief: Following The Khronos Group Inc. tutorial (https://vulkan-tutorial.com/).
 */

// glfw.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// glm.
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std.
#include <array>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

/** GLFW. **/
const int WIDTH = 800;
const int HEIGHT = 600;

/** GLM. **/

struct Vertex
{
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription getBindingDescription( )
  {
    VkVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array< VkVertexInputAttributeDescription, 2 > getAttributeDescriptions( )
  {
    std::array< VkVertexInputAttributeDescription, 2 > attributeDescriptions = {};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof( Vertex, pos );

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof( Vertex, color );

    return attributeDescriptions;
  }
};

// Now we have a rectangle.
const std::vector< Vertex > vertices =
{
  { {-0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
  { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
  { { 0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
  { {-0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } }
};

const std::vector< uint16_t > indices =
{ 0, 1, 2, 2, 3, 0 };

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

/** Shaders. They must be in *.spv Vulkan format. **/
std::string triangleVS( "/home/jguerrero/opt/Lava/spvs/uniformBuffersByJGM_vert.spv" );
std::string triangleFS( "/home/jguerrero/opt/Lava/spvs/uniformBuffersByJGM_frag.spv" );

/** Instance-related. **/

// Using validation layers only if debugging.
const std::vector< const char* > validationLayers =
{
  "VK_LAYER_LUNARG_standard_validation"
};
// NDEBUG C++ macro that stands for "no debug".
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

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

/** Surface-related. **/

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector< VkSurfaceFormatKHR > formats;
  std::vector< VkPresentModeKHR > presentModes;
};

/** Device-related. **/

// Device extensions.
const std::vector< const char* > deviceExtensions =
{
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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
    // Auxiliar functions.
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

    void createInstance( void )
    {
      if( enableValidationLayers && !checkValidationLayerSupport( ) )
      {
        throw std::runtime_error( "validation layers requested, but not available!" );
      }

      // VkApplicationInfo struct. Optional.
      VkApplicationInfo appInfo = {};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = "Hello Triangle";
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

      return indices.isComplete( ) && extensionsSupported && swapChainAdequate;
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

    void createImageViews( void )
    {
      swapChainImageViews.resize( swapChainImages.size( ) );

      for( size_t i = 0; i < swapChainImages.size( ); i++ )
      {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        // Here you can do some permutations.
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // VkImageView treatment, mipmaps or layers.
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if( vkCreateImageView( device, &createInfo, nullptr, &swapChainImageViews[i] ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to create image views!" );
        }
      }
    }

    void createDescriptorSetLayout( void )
    {
      VkDescriptorSetLayoutBinding uboLayoutBinding = {};
      uboLayoutBinding.binding = 0;
      uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.descriptorCount = 1;
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
      uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

      VkDescriptorSetLayoutCreateInfo layoutInfo = {};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = 1;
      layoutInfo.pBindings = &uboLayoutBinding;

      if( vkCreateDescriptorSetLayout( device, &layoutInfo, nullptr, &descriptorSetLayout ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create descriptor set layout!" );
      }


    }

    void createGraphicsPipeline( void )
    {
      // Getting shaders byte array.
      auto vertShaderCode = readFile( triangleVS );
      auto fragShaderCode = readFile( triangleFS );

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
      auto bindingDescription = Vertex::getBindingDescription( );
      auto attributeDescriptions = Vertex::getAttributeDescriptions( );
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
      // VkPipelineDepthStencilStateCreateInfo depthStencil = nullptr;

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
      pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
      pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
      pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

      if( vkCreatePipelineLayout( device, &pipelineLayoutInfo, nullptr, &pipelineLayout ) != VK_SUCCESS )
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
      pipelineInfo.pDepthStencilState = nullptr; // Optional
      pipelineInfo.pColorBlendState = &colorBlending;
      pipelineInfo.pDynamicState = nullptr; // Optional
      pipelineInfo.layout = pipelineLayout;
      pipelineInfo.renderPass = renderPass;
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
      pipelineInfo.basePipelineIndex = -1; // Optional

      if( vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create graphics pipeline!" );
      }

      // Destroying VkShaderModule(s).
      vkDestroyShaderModule( device, fragShaderModule, nullptr );
      vkDestroyShaderModule( device, vertShaderModule, nullptr );
    }

    void createRenderPass( void )
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

      VkAttachmentReference colorAttachmentRef = {};
      colorAttachmentRef.attachment = 0;
      colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      // A pass can have multiple subpasses.
      VkSubpassDescription subpass = {};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &colorAttachmentRef;

      VkRenderPassCreateInfo renderPassInfo = {};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassInfo.attachmentCount = 1;
      renderPassInfo.pAttachments = &colorAttachment;
      renderPassInfo.subpassCount = 1;
      renderPassInfo.pSubpasses = &subpass;

      // Subpass dependencies.
      VkSubpassDependency dependency = {};
      dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
      dependency.dstSubpass = 0;
      dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.srcAccessMask = 0;
      dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

      renderPassInfo.dependencyCount = 1;
      renderPassInfo.pDependencies = &dependency;

      // Finally creating the render pass.
      if( vkCreateRenderPass( device, &renderPassInfo, nullptr, &renderPass ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create render pass!" );
      }
    }

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

    void createFramebuffers( void )
    {
      swapChainFramebuffers.resize( swapChainImageViews.size( ) );

      // Creating a framebuffer foreach image view.
      for( size_t i = 0; i < swapChainImageViews.size( ); i++ )
      {
        VkImageView attachments[] =
        {
          swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if( vkCreateFramebuffer( device, &framebufferInfo, nullptr, &swapChainFramebuffers[i] ) != VK_SUCCESS )
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

    void createVertexBuffer( void )
    {
      VkDeviceSize bufferSize = sizeof( vertices[0] ) * vertices.size( );

      // Staging buffer.
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      createBuffer( bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory );

      // Data mapping.
      void* data;
      vkMapMemory( device, stagingBufferMemory, 0, bufferSize, 0, &data );
      memcpy( data, vertices.data( ), (size_t) bufferSize );
      vkUnmapMemory( device, stagingBufferMemory );

      // Actual vertex buffer.
      createBuffer( bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    vertexBuffer,
                    vertexBufferMemory );

      copyBuffer( stagingBuffer, vertexBuffer, bufferSize );

      vkDestroyBuffer( device, stagingBuffer, nullptr );
      vkFreeMemory( device, stagingBufferMemory, nullptr );
    }

    void createIndexBuffer( void )
    {
      VkDeviceSize bufferSize = sizeof( indices[0] ) * indices.size( );

      // Staging buffer.
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      createBuffer( bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory );

      // Data mapping.
      void* data;
      vkMapMemory( device, stagingBufferMemory, 0, bufferSize, 0, &data );
      memcpy( data, indices.data( ), (size_t) bufferSize );
      vkUnmapMemory( device, stagingBufferMemory );

      // Actual index buffer.
      createBuffer( bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    indexBuffer,
                    indexBufferMemory );

      copyBuffer( stagingBuffer, indexBuffer, bufferSize );

      vkDestroyBuffer( device, stagingBuffer, nullptr );
      vkFreeMemory( device, stagingBufferMemory, nullptr );
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

    void createDescriptorPool( void )
    {
      VkDescriptorPoolSize poolSize = {};
      poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSize.descriptorCount = 1;

      VkDescriptorPoolCreateInfo poolInfo = {};
      poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolInfo.poolSizeCount = 1;
      poolInfo.pPoolSizes = &poolSize;
      poolInfo.maxSets = 1;

      if( vkCreateDescriptorPool( device, &poolInfo, nullptr, &descriptorPool ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create descriptor pool!" );
      }
    }

    void createDescriptorSet( void )
    {
      VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
      VkDescriptorSetAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = descriptorPool;
      allocInfo.descriptorSetCount = 1;
      allocInfo.pSetLayouts = layouts;

      if( vkAllocateDescriptorSets( device, &allocInfo, &descriptorSet ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to allocate descriptor set!" );
      }

      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = uniformBuffer;
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof( UniformBufferObject );

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = descriptorSet;
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;
      descriptorWrite.pImageInfo = nullptr; // Optional
      descriptorWrite.pTexelBufferView = nullptr; // Optional

      vkUpdateDescriptorSets( device, 1, &descriptorWrite, 0, nullptr );
    }

    void copyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size )
    {
      VkCommandBufferAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool = commandPool;
      allocInfo.commandBufferCount = 1;

      VkCommandBuffer commandBuffer;
      vkAllocateCommandBuffers( device, &allocInfo, &commandBuffer );

      // BEGIN.
      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      vkBeginCommandBuffer( commandBuffer, &beginInfo );

      VkBufferCopy copyRegion = {};
      copyRegion.srcOffset = 0; // Optional
      copyRegion.dstOffset = 0; // Optional
      copyRegion.size = size;
      vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

      vkEndCommandBuffer(commandBuffer);
      // END.

      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffer;

      vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
      vkQueueWaitIdle( graphicsQueue );

      vkFreeCommandBuffers( device, commandPool, 1, &commandBuffer );
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
      commandBuffers.resize( swapChainFramebuffers.size( ) );

      VkCommandBufferAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = commandPool;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = ( uint32_t ) commandBuffers.size( );

      if( vkAllocateCommandBuffers( device, &allocInfo, commandBuffers.data( ) ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to allocate command buffers!" );
      }

      for( size_t i = 0; i < commandBuffers.size( ); i++ )
      {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        vkBeginCommandBuffer( commandBuffers[i], &beginInfo );

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass( commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

          vkCmdBindPipeline( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );

          // Working with buffers.
          VkBuffer vertexBuffers[] = { vertexBuffer };
          VkDeviceSize offsets[] = { 0 };
          vkCmdBindVertexBuffers( commandBuffers[i], 0, 1, vertexBuffers, offsets );

          vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

          vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

          vkCmdDrawIndexed(
            commandBuffers[i],
            static_cast< uint32_t >( indices.size( ) ),
            1, 0, 0, 0 );

        vkCmdEndRenderPass( commandBuffers[i] );

        if( vkEndCommandBuffer( commandBuffers[i] ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to record command buffer!" );
        }
      }
    }

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

    void recreateSwapChain( void )
    {
      vkDeviceWaitIdle( device );

      cleanupSwapChain( );

      createSwapChain( );
      createImageViews( );
      createRenderPass( );
      createGraphicsPipeline( );
      createFramebuffers( );
      createCommandBuffers( );
    }

    // Static funcions.

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

    static void onWindowResized( GLFWwindow* window, int width, int height )
    {
      if( width == 0 || height == 0 ) return;

      HelloTriangleApplication* app =
        reinterpret_cast< HelloTriangleApplication* >( glfwGetWindowUserPointer( window ) );
      // Vulkan may require this.
      app->recreateSwapChain( );
    }

    // Run functions.
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
      createInstance( );
      setupDebugCallback( );
      createSurface( );
      pickPhysicalDevice( );
      createLogicalDevice( );
      createSwapChain( );
      createImageViews( );
      createRenderPass( );
      createDescriptorSetLayout( );
      createGraphicsPipeline( );
      createFramebuffers( );
      createCommandPool( );
      // Buffer-related.
      createVertexBuffer( );
      createIndexBuffer( );
      createUniformBuffer( );
      createDescriptorPool( );
      createDescriptorSet( );
      createCommandBuffers( );
      createSemaphores( );
    }

    void mainLoop( void )
    {
      while( !glfwWindowShouldClose( window ) )
      {
        glfwPollEvents( );

        updateUniformBuffer( );
        drawFrame( );
      }
      vkDeviceWaitIdle(device);
    }

    void updateUniformBuffer( void )
    {
      static auto startTime = std::chrono::high_resolution_clock::now( );

      auto currentTime = std::chrono::high_resolution_clock::now( );
      float time = std::chrono::duration< float, std::chrono::seconds::period >
                   ( currentTime - startTime ).count( );

      UniformBufferObject ubo = {};
      ubo.model = glm::rotate( glm::mat4( 1.0f ),
                               time * glm::radians( 90.0f ),
                               glm::vec3( 0.0f, 0.0f, 1.0f ) );
      ubo.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ),
                              glm::vec3( 0.0f, 0.0f, 0.0f ),
                              glm::vec3( 0.0f, 0.0f, 1.0f ) );
      ubo.proj = glm::perspective( glm::radians(45.0f),
                                   swapChainExtent.width /
                                   (float) swapChainExtent.height,
                                   0.1f, 10.0f );
      // GLM was written for OpenGL. Vulkan has the Y inverted.
      ubo.proj[1][1] *= -1;

      void* data;
      vkMapMemory( device, uniformBufferMemory, 0, sizeof( ubo ), 0, &data );
      memcpy( data, &ubo, sizeof( ubo ) );
      vkUnmapMemory( device, uniformBufferMemory );
    }

    void cleanupSwapChain( void )
    {
      for( size_t i = 0; i < swapChainFramebuffers.size( ); i++ )
      {
        vkDestroyFramebuffer( device, swapChainFramebuffers[i], nullptr );
      }

      // Deallocating CB.
      vkFreeCommandBuffers( device, commandPool,
                            static_cast< uint32_t >( commandBuffers.size( ) ), commandBuffers.data( ) );

      vkDestroyPipeline( device, graphicsPipeline, nullptr );
      vkDestroyPipelineLayout( device, pipelineLayout, nullptr );
      vkDestroyRenderPass( device, renderPass, nullptr );

      for( size_t i = 0; i < swapChainImageViews.size( ); i++ )
      {
        vkDestroyImageView( device, swapChainImageViews[i], nullptr );
      }

      vkDestroySwapchainKHR( device, swapChain, nullptr );
    }

    void cleanup( void )
    {
      // Swapchain, ImageView(s), RenderPass, Pipeline, CB(s), FB(s).
      cleanupSwapChain( );

      // DescriptorPool(s).
      vkDestroyDescriptorPool(device, descriptorPool, nullptr);

      // DescriptorSetLayout(s).
      vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

      // Buffer(s).
      vkDestroyBuffer(device, uniformBuffer, nullptr);
      vkFreeMemory(device, uniformBufferMemory, nullptr);

      vkDestroyBuffer(device, indexBuffer, nullptr);
      vkFreeMemory(device, indexBufferMemory, nullptr);

      vkDestroyBuffer(device, vertexBuffer, nullptr);
      vkFreeMemory(device, vertexBufferMemory, nullptr);

      // Semaphore(s).
      vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
      vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);

      vkDestroyCommandPool( device, commandPool, nullptr );

      // cleanupSwapChain( ) may be here too.

      vkDestroyDevice( device, nullptr );
      DestroyDebugReportCallbackEXT( instance, callback, nullptr );
      vkDestroySurfaceKHR( instance, surface, nullptr );
      vkDestroyInstance( instance, nullptr );

      glfwDestroyWindow( window );

      glfwTerminate( );
    }

    /** Attributes. **/

    // GLFW.
    GLFWwindow* window;

    // Instance-related.
    VkInstance instance;
    VkDebugReportCallbackEXT callback;

    // KHR-related.
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    std::vector< VkImage > swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector< VkImageView > swapChainImageViews;

    // Pipeline-related.
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    // Drawing-related.
    std::vector< VkFramebuffer > swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector< VkCommandBuffer > commandBuffers;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    // Device-related.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // It will be destroyed with VkInstance.
    VkDevice device;

    // Buffer-related.
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    // Queue-related.
    VkQueue graphicsQueue;
    VkQueue presentQueue;
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
