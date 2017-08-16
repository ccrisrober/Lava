#ifndef __VKLAVA_VULKANRENDERAPI__
#define __VKLAVA_VULKANRENDERAPI__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

#include <vector>
#include "VulkanDevice.h"
#include "RenderWindow.h"
#include "VulkanSwapChain.h"

const int WIDTH = 800;
const int HEIGHT = 600;



#include <glm/glm.hpp>
#include <array>

namespace vklava
{
  struct Vertex
  {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription( )
    {
      VkVertexInputBindingDescription bindingDescription = { };
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof( Vertex );
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions( )
    {
      std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = { };

      // inPosition
      attributeDescriptions[ 0 ].binding = 0;
      attributeDescriptions[ 0 ].location = 0;
      attributeDescriptions[ 0 ].format = VK_FORMAT_R32G32_SFLOAT;  // vec2
      attributeDescriptions[ 0 ].offset = offsetof( Vertex, pos );

      // inColor
      attributeDescriptions[ 1 ].binding = 0;
      attributeDescriptions[ 1 ].location = 1;
      attributeDescriptions[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
      attributeDescriptions[ 1 ].offset = offsetof( Vertex, color );

      return attributeDescriptions;
    }
  };

  /*const std::vector<Vertex> vertices = {
    //{ { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    //{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
    //{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },

    // first triangle
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },

    // second triangle
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
  };*/

  const std::vector<Vertex> vertices = {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
  };

  const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
  };



  class VulkanRenderAPI
  {
  public:
    static void onWindowResized( GLFWwindow* window, int width, int height )
    {
      if ( width == 0 || height == 0 ) return;

      VulkanRenderAPI* app = reinterpret_cast<VulkanRenderAPI*>
        ( glfwGetWindowUserPointer( window ) );
      app->resize( );
    }
    void resize( )
    {
      getPresentDevice( )->waitIdle( );
      
      std::cout << "REGENERATE SWAP CHAIN" << std::endl;
    }
    VulkanRenderAPI( void );
    ~VulkanRenderAPI( void );
    void initialize( void );
    void run( void )
    {
      while ( !glfwWindowShouldClose( _window ) )
      {
        glfwPollEvents( );
        drawFrame( );
      }
      getPresentDevice( )->waitIdle( );
    }
    VulkanSemaphore* imageAvailableSemaphore;
    VulkanSemaphore* renderFinishedSemaphore;
    void drawFrame( void )
    {
      uint32_t imageIndex;
      VkSwapchainKHR swapChain = _renderWindow->_swapChain->getHandle( );
      VkResult result = 
        vkAcquireNextImageKHR( getPresentDevice( )->getLogical( ), swapChain,
        std::numeric_limits<uint64_t>::max( ), imageAvailableSemaphore->getHandle( ),
        VK_NULL_HANDLE, &imageIndex );

      VkSubmitInfo submitInfo = { };
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

      VkSemaphore waitSemaphores[] = { imageAvailableSemaphore->getHandle( ) };
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = waitSemaphores;
      submitInfo.pWaitDstStageMask = waitStages;

      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffers[ imageIndex ];

      VkSemaphore signalSemaphores[] = { renderFinishedSemaphore->getHandle( ) };
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = signalSemaphores;

      VulkanQueue* graphicsQueue = getPresentDevice( )->getQueue(
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
    VkInstance getInstance( void ) const
    {
      return _instance;
    }

    GLFWwindow* getWindow( void ) const
    {
      return _window;
    }

    std::shared_ptr<VulkanDevice> getPresentDevice( void ) const
    {
      return _primaryDevices.front( );
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
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;


    //VkSurfaceKHR _surface;
    std::shared_ptr<RenderWindow> _renderWindow;

    VkCommandPool commandPool;

    // Vertex buffer
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    // Index buffer
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkCommandBuffer> commandBuffers;
    // <MOVE TO ANOTHER CLASS \\



    VkInstance _instance;
    GLFWwindow* _window;
  };
}

#endif /* __VKLAVA_VULKANRENDERAPI__ */