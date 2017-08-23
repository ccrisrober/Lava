#ifndef __VKLAVA_VULKANRENDERAPI__
#define __VKLAVA_VULKANRENDERAPI__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <vector>
#include "VulkanDevice.h"
#include "RenderWindow.h"
#include "VulkanSwapChain.h"

const int WIDTH = 800;
const int HEIGHT = 600;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <array>

#include "noncopyable.hpp"
#include "RenderAPICapabilites.h"

#include "VulkanFramebuffer.h"

namespace lava
{
  class VulkanSamplerState;

  template< class E >
  class Singleton
  {
  private:
    static E _dummyInstance;
  public:
    static E* getInstance( void )
    {
      static E instance;
      return &instance;
    }
  };
  struct Vertex
  {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription( )
    {
      VkVertexInputBindingDescription bindingDescription;
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof( Vertex );
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions( )
    {
      std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

      attributeDescriptions[ 0 ].binding = 0;
      attributeDescriptions[ 0 ].location = 0;
      attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );

      attributeDescriptions[ 1 ].binding = 0;
      attributeDescriptions[ 1 ].location = 1;
      attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[ 1 ].offset = offsetof( Vertex, normal );

      attributeDescriptions[ 2 ].binding = 0;
      attributeDescriptions[ 2 ].location = 2;
      attributeDescriptions[ 2 ].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[ 2 ].offset = offsetof( Vertex, texCoord );

      return attributeDescriptions;
    }
  };

  struct UniformBufferObject
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

  const float side = 1.0f;
  const float side2 = side / 2.0f;
  const std::vector<Vertex> vertices = {
    { { -side2, -side2, side2 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
    { { side2, -side2, side2 }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
    { { side2, side2, side2 }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
    { { -side2, side2, side2 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },


    { { side2, -side2, side2 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { side2, -side2, -side2 }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { side2, side2, -side2 }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
    { { side2, side2, side2 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },


    { { -side2, -side2, -side2 }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { { -side2, side2, -side2 }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
    { { side2, side2, -side2 }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
    { { side2, -side2, -side2 }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },


    { { -side2, -side2, side2 }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -side2, side2, side2 }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -side2, side2, -side2 }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
    { { -side2, -side2, -side2 }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },


    { { -side2, -side2, side2 }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -side2, -side2, -side2 }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
    { { side2, -side2, -side2 }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { side2, -side2, side2 }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },


    { { -side2, side2, side2 }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { side2, side2, side2 }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
    { { side2, side2, -side2 }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { -side2, side2, -side2 }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
  };

  const std::vector<uint16_t> indices = {
    0, 1, 2, 0, 2, 3,
    4, 5, 6, 4, 6, 7,
    8, 9, 10, 8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23
  };



  class VulkanRenderAPI : public Singleton<VulkanRenderAPI>
  {
  public:
    RenderAPICapabilities caps;
    static void onWindowResized( GLFWwindow* window, int width, int height )
    {
      if ( width == 0 || height == 0 ) return;

      VulkanRenderAPI* app = reinterpret_cast<VulkanRenderAPI*>
        ( glfwGetWindowUserPointer( window ) );
      app->resize( width, height );
    }
    void resize( uint32_t width, uint32_t height )
    {
      _getPresentDevice( )->waitIdle( );
      
      std::cout << "REGENERATE SWAP CHAIN" << std::endl;
      _renderWindow->resize( width , height );
    }
    VulkanRenderAPI( void );
    ~VulkanRenderAPI( void );
    void initialize( void );

    void updateUniformBuffer( )
    {
      VkDevice logicalDevice = _getPresentDevice( )->getLogical( );
      static auto startTime = std::chrono::high_resolution_clock::now( );

      auto currentTime = std::chrono::high_resolution_clock::now( );
      float time = std::chrono::duration_cast<std::chrono::milliseconds>( 
        currentTime - startTime ).count( ) / 1000.0f;

      UniformBufferObject ubo = { };
      ubo.model = glm::rotate( glm::mat4( ), time * glm::radians( 90.0f ), 
        glm::vec3( 1.0f, 0.0f, 0.0f ) );
      ubo.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), 
        glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
      ubo.proj = glm::perspective( glm::radians( 45.0f ), 
        _renderWindow->_swapChain->getWidth( ) / 
        ( float ) _renderWindow->_swapChain->getHeight( ), 0.1f, 10.0f );
      ubo.proj[ 1 ][ 1 ] *= -1;

      void* data;
      vkMapMemory( logicalDevice, uniformBufferVS.memory, 0, sizeof( ubo ), 0, &data );
      memcpy( data, &ubo, sizeof( ubo ) );
      vkUnmapMemory( logicalDevice, uniformBufferVS.memory );

      float timeValue = glfwGetTime( );
      float greenValue = sin( timeValue ) / 2.0f + 0.5f;
      pushConstants[ 1 ].g = greenValue;

      std::cout << greenValue << std::endl;
    }
    void run( void )
    {
      while ( !glfwWindowShouldClose( _window ) )
      {
        glfwPollEvents( );
        updateUniformBuffer( );
        drawFrame( );
      }
      _getPresentDevice( )->waitIdle( );
    }

    // Synchronization semaphores
    struct {
      // Swap chain image presentation
      VulkanSemaphore* presentComplete;
      // Command buffer submission and execution
      VulkanSemaphore* renderComplete;
    } semaphores;

    void drawFrame( void )
    {
      uint32_t imageIndex;
      VkSwapchainKHR swapChain = _renderWindow->_swapChain->getHandle( );
      VkResult result = 
        vkAcquireNextImageKHR( _getPresentDevice( )->getLogical( ), swapChain,
        std::numeric_limits<uint64_t>::max( ), semaphores.presentComplete->getHandle( ),
        VK_NULL_HANDLE, &imageIndex );

      VkSubmitInfo submitInfo = { };
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

      VkSemaphore waitSemaphores[] = { semaphores.presentComplete->getHandle( ) };
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = waitSemaphores;
      submitInfo.pWaitDstStageMask = waitStages;

      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffers[ imageIndex ];

      VkSemaphore signalSemaphores[] = { semaphores.renderComplete->getHandle( ) };
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = signalSemaphores;

      VulkanQueue* graphicsQueue = _getPresentDevice( )->getQueue(
        GpuQueueType::GPUT_GRAPHICS, 0 );

        if ( vkQueueSubmit( graphicsQueue->getQueue( ), 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to submit draw command buffer!" );
        }

        VkPresentInfoKHR presentInfo = { };
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR( graphicsQueue->getQueue( ), &presentInfo );

        if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
        {
          std::cout << "recreateSwapChain" << std::endl;
          //recreateSwapChain( );
        }
        else if ( result != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to present swap chain image!" );
        }

        graphicsQueue->waitIdle( );
    }

    // Returns the internal Vulkan instance object.
    VkInstance _getInstance( void ) const
    {
      return _instance;
    }

    GLFWwindow* getWindow( void ) const
    {
      return _window;
    }


    void __init__( );

    // Returns the primary device that supports swap chain present operations
    VulkanDevicePtr _getPresentDevice( void ) const
    {
      return _primaryDevices.front( );
    }
    /** Returns a Vulkan device at the specified index. Must be in range [0, _getNumDevices()) */
    VulkanDevicePtr _getDevice( uint32_t idx ) const
    {
      return _devices[ idx ];
    }
    /** Gets the total number of Vulkan compatible devices available on this system. */
    uint32_t _getNumDevices( ) const
    {
      return ( uint32_t ) _devices.size( );
    }

    void cleanup( void );
    void cleanupSwapChain( void );
  protected:
#ifndef NDEBUG
    VkDebugReportCallbackEXT _debugCallback;
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = nullptr;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;
#endif
    std::vector<VulkanDevicePtr> _devices;
    std::vector<VulkanDevicePtr> _primaryDevices;


    bool checkValidationLayerSupport( const std::vector<const char*>& validationLayers );


    // MOVE TO ANOTHER CLASS
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;


    //VkSurfaceKHR _surface;
    std::shared_ptr<RenderWindow> _renderWindow;

    VkCommandPool commandPool;

    // Depth texture
    //VkImage depthImage;
    //VkDeviceMemory depthImageMemory;
    //VkImageView depthImageView;
    struct
    {
      VkImage image;
      VkDeviceMemory mem;
      VkImageView view;
    } depthStencilTex;

    // Texture image
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VulkanSamplerState* textureSampler; //VkSampler textureSampler;

    // Vertex buffer
    struct {
      VkDeviceMemory memory;	// Handle to the device memory for this buffer
      VkBuffer buffer;				// Handle to the Vulkan buffer object that the memory is bound to
    } verticesBuffer;

    // Index buffer
    struct
    {
      VkDeviceMemory memory;
      VkBuffer buffer;
      uint32_t count;
    } indicesBuffer;

    // Uniform buffer block object
    struct {
      VkDeviceMemory memory;
      VkBuffer buffer;
      VkDescriptorBufferInfo descriptor;
    } uniformBufferVS;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    std::vector<VkCommandBuffer> commandBuffers;
    // <MOVE TO ANOTHER CLASS \\



    VkInstance _instance;
    GLFWwindow* _window;



    VkFormat  findSupportedFormat( const std::vector<VkFormat>& candidates,
      VkImageTiling tiling, VkFormatFeatureFlags features )
    {
      std::shared_ptr<VulkanDevice> presentDevice = _getPresentDevice( );
      VkPhysicalDevice physicalDevice = presentDevice->getPhysical( );
      for ( VkFormat format : candidates )
      {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties( physicalDevice, format, &props );

        if ( tiling == VK_IMAGE_TILING_LINEAR && (
          props.linearTilingFeatures & features ) == features )
        {
          return format;
        }
        else if ( tiling == VK_IMAGE_TILING_OPTIMAL && (
          props.optimalTilingFeatures & features ) == features )
        {
          return format;
        }
      }

      throw std::runtime_error( "failed to find supported format!" );
    };

    bool hasStencilComponent( VkFormat format )
    {
      return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }
    VkFormat findDepthFormat( )
    {
      return findSupportedFormat(
      { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
      );
    };
    VkCommandBuffer beginSingleTimeCommands( )
    {
      VkCommandBufferAllocateInfo allocInfo = { };
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool = commandPool;
      allocInfo.commandBufferCount = 1;

      VkCommandBuffer commandBuffer;
      VkDevice logicalDevice = _getPresentDevice( )->getLogical( );
      vkAllocateCommandBuffers( logicalDevice, &allocInfo, &commandBuffer );

      VkCommandBufferBeginInfo beginInfo = { };
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      vkBeginCommandBuffer( commandBuffer, &beginInfo );

      return commandBuffer;
    }

    void endSingleTimeCommands( VkCommandBuffer commandBuffer )
    {
      vkEndCommandBuffer( commandBuffer );

      VkSubmitInfo submitInfo = { };
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffer;

      VulkanQueue* graphicsQueue = _getPresentDevice( )->getQueue(
        GpuQueueType::GPUT_GRAPHICS, 0 );

      vkQueueSubmit( graphicsQueue->getQueue( ), 1, &submitInfo, VK_NULL_HANDLE );
      graphicsQueue->waitIdle( );

      VkDevice logicalDevice = _getPresentDevice( )->getLogical( );
      vkFreeCommandBuffers( logicalDevice, commandPool, 1, &commandBuffer );
    }
    VkImageView createImageView( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags )
    {
      VkImageViewCreateInfo viewInfo = { };
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
      VkDevice logicalDevice = _getPresentDevice( )->getLogical( );
      if ( vkCreateImageView( logicalDevice, &viewInfo, nullptr, &imageView ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create texture image view!" );
      }

      return imageView;
    }

    void createImage( uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory )
    {
      VkImageCreateInfo imageInfo = { };
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

      VkDevice logicalDevice = _getPresentDevice( )->getLogical( );
      if ( vkCreateImage( logicalDevice, &imageInfo, nullptr, &image ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create image!" );
      }

      VkMemoryRequirements memRequirements;
      vkGetImageMemoryRequirements( logicalDevice, image, &memRequirements );

      imageMemory = _getPresentDevice( )->allocateMemReqMemory( memRequirements, properties );

      vkBindImageMemory( logicalDevice, image, imageMemory, 0 );
    }
    void transitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout )
    {
      VkCommandBuffer commandBuffer = beginSingleTimeCommands( );

      VkImageMemoryBarrier barrier = { };
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.oldLayout = oldLayout;
      barrier.newLayout = newLayout;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.image = image;

      if ( newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
      {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if ( hasStencilComponent( format ) )
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

      if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
      {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      }
      else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
      {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      }
      else if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
      {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

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

    
    VkPipelineDynamicStateCreateInfo _dynamicStateInfo;
    VkDynamicState _dynamicStates[ 3 ];


    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding( 
      VkDescriptorType type, VkShaderStageFlagBits stage, uint32_t index )
    {
      VkDescriptorSetLayoutBinding layoutBinding;
      layoutBinding.binding = index;
      layoutBinding.descriptorCount = 1;
      layoutBinding.descriptorType = type;
      layoutBinding.pImmutableSamplers = nullptr;
      layoutBinding.stageFlags = stage;

      return layoutBinding;
    }

    std::array<glm::vec4, 3> pushConstants;

  protected:
    void initCapabilities( void );
  };
}

#endif /* __VKLAVA_VULKANRENDERAPI__ */