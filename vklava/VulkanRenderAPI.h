#pragma once

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

const std::vector<Vertex> vertices = {
  { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
  { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
  { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }

  /*// first triangle
  { { 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, },  // top right
  { { 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, },  // bottom right
  { { -0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, },  // top left 
  // second triangle
  { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, },  // bottom right
  { { -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, },  // bottom left
  { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }  }  // top left*/
};



class VulkanRenderAPI
{
public:
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
  }
  VulkanSemaphore* imageAvailableSemaphore;
  VulkanSemaphore* renderFinishedSemaphore;
  void drawFrame( void )
  {
    uint32_t imageIndex;
    VkSwapchainKHR swapChain = _renderWindow->_swapChain->getSwapChain( );
    vkAcquireNextImageKHR( getPresentDevice( )->getLogical( ), swapChain,
      std::numeric_limits<uint64_t>::max( ), imageAvailableSemaphore->getHandle( ), 
      VK_NULL_HANDLE, &imageIndex );

    VkSubmitInfo submitInfo = { };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[ ] = { imageAvailableSemaphore->getHandle( ) };
    VkPipelineStageFlags waitStages[ ] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[ imageIndex ];

    VkSemaphore signalSemaphores[ ] = { renderFinishedSemaphore->getHandle( ) };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkQueue graphicsQueue = getPresentDevice( )->_queueInfos[ GpuQueueType::GPUT_GRAPHICS ].queues.front( )->getQueue( );

    if ( vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to submit draw command buffer!" );
    }

    VkPresentInfoKHR presentInfo = { };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[ ] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR( graphicsQueue, &presentInfo );

    vkQueueWaitIdle( graphicsQueue );
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
protected:
#ifndef NDEBUG
  VkDebugReportCallbackEXT _debugCallback;
  PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = nullptr;
  PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;
#endif
  std::vector<VulkanDevicePtr> _devices;
  std::vector<VulkanDevicePtr> _primaryDevices;


  bool checkValidationLayerSupport( const std::vector<const char*>& validationLayers )
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
          //break;
        }
        std::cout << layerProperties.layerName << std::endl;
      }

      if ( !layerFound )
      {
        return false;
      }
    }

    return true;
  }


  // MOVE TO ANOTHER CLASS
  std::vector<VkFramebuffer> swapChainFramebuffers;

  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;
 

  //VkSurfaceKHR _surface;
  std::shared_ptr<RenderWindow> _renderWindow;

  VkCommandPool commandPool;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;

  std::vector<VkCommandBuffer> commandBuffers;
  // <MOVE TO ANOTHER CLASS \\



  VkInstance _instance;
  GLFWwindow* _window;
};
