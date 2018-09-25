/**
 * Copyright (c) 2017 - 2018, Pompeii
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#pragma once

#include <pompeii/pompeii.h>
#include <glfwPompeii/api.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace pompeii
{
  namespace glfw
  {
    class VulkanWindowRenderer
    {
    public:
      GLFWPOMPEII_API
      virtual ~VulkanWindowRenderer( void ) { }

      /**
      * Method called when creating renderer's resources
      */
      GLFWPOMPEII_API
      virtual void initResources( void ) { }
      GLFWPOMPEII_API
      virtual void initSwapChainResources( void ) { }
      GLFWPOMPEII_API
      virtual void releaseSwapChainResources( void ) { }
      /**
      * Method called when renderer's resources must be released
      */
      GLFWPOMPEII_API
      virtual void releaseResources( void ) { }
      /**
      * Method called when the draw calls for the next frame are to be added
      *   to the command buffer
      */
      GLFWPOMPEII_API
      virtual void nextFrame( void ) = 0;

      //virtual void physicalDeviceLost( void );
      //virtual void logicalDeviceLost( void );
    };

    class VulkanWindow
    {
    private:
      class Engine
      {
      public:
        struct CreateInfo
        {
          std::string appInfo;
          bool enableValidationLayers = false;
          bool enableRenderdoc = false;
          std::set<std::string> requiredInstanceExtensions;
          std::set<std::string> requiredDeviceExtensions;

          CreateInfo( void ) = default;
        };
        Engine( const CreateInfo &info );
        ~Engine( void );
        const vk::Instance& GetVkInstance( void )	const { return instance; }
      private:
        void createInstance( void );

        const CreateInfo info;

        vk::Instance instance;
      };
    protected:
      GLFWPOMPEII_API
      virtual void setupRenderPass( void );
      GLFWPOMPEII_API
      virtual void setupFramebuffer( void );
	  GLFWPOMPEII_API
	  virtual void getEnabledFeatures(void) { }

	  // Stores the features available on the selected physical device (for e.g. checking if a feature is available)
	  vk::PhysicalDeviceFeatures deviceFeatures;
	  /**
	  * Set of physical device features to be enabled for this example (must be set in the derived constructor)
	  *
	  * @note By default no phyiscal device features are enabled
	  */
	  vk::PhysicalDeviceFeatures enabledFeatures;
    protected:
      GLFWPOMPEII_API
      virtual bool setupPipelineCache( void );
    private:
      GLFWwindow* window;
      static void OnWindowResized( GLFWwindow *window, int width, int height );

      void initVulkan( void );
      void cleanupVulkan( void );
      void recreateSwapchain( void );
      bool _initialized;

      std::shared_ptr< Instance > _instance = nullptr;
      std::shared_ptr< PhysicalDevice > _physicalDevice = nullptr;

      std::shared_ptr< Device > _device = nullptr;

      std::shared_ptr< Surface > surface = nullptr;


      vk::SurfaceFormatKHR _surfaceFormat;
      vk::Format _dsFormat;

      uint32_t imageIdx;

      std::vector<std::string > _requestedDeviceExts;
      uint32_t _gfxQueueFamilyIdx;
      uint32_t _presQueueFamilyIdx;
      std::shared_ptr< Queue > _gfxQueue;
      std::shared_ptr< Queue > _presQueue;

      std::vector<std::shared_ptr< CommandBuffer > > cmds;

      std::shared_ptr< CommandPool > _cmdPool;

      bool _framePending = false;
      bool _frameGrabbing = false;
      //QImage _frameGrabTargetImage;
      std::shared_ptr< Image > frameGrabImage = nullptr;
    protected:
      std::shared_ptr< PipelineCache > _pipelineCache;
    private:
      vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

      std::shared_ptr< Semaphore > _renderComplete = nullptr;
      VulkanWindowRenderer* renderer = nullptr;

    protected:
      GLFWPOMPEII_API
      virtual VulkanWindowRenderer* createRenderer( void );
    public:
      GLFWPOMPEII_API
      void requestUpdate( void )
      {
        glfwPollEvents( );
      }

      //GLFWPOMPEII_API
      //virtual void setVkInstance( const vk::Instance& instance );
      GLFWPOMPEII_API
      explicit VulkanWindow( int width, int height, 
        const std::string& title, bool enableLayers );
      GLFWPOMPEII_API
      VulkanWindow( const VulkanWindow& ) = delete;
      GLFWPOMPEII_API
      VulkanWindow( VulkanWindow&& ) = delete;
      GLFWPOMPEII_API
      virtual ~VulkanWindow( void );

      GLFWPOMPEII_API
      VulkanWindow& operator=( const VulkanWindow& ) = delete;
      GLFWPOMPEII_API
      VulkanWindow& operator=( VulkanWindow&& ) = delete;

      GLFWPOMPEII_API
      const std::shared_ptr< PipelineCache > pipelineCache( void )
      {
        return _pipelineCache;
      }
    public:
      GLFWPOMPEII_API
      void frameReady( std::shared_ptr<Semaphore> sem = nullptr );

     // GLFWPOMPEII_API
      //QImage grab( void );

      GLFWPOMPEII_API
      bool supportGrab( void ) const
      {
        return _swapchain ? _swapchain->swapchainSupportsReadBack( ) : false;
      }

      GLFWPOMPEII_API
      GLFWwindow* getWindow( void ) const { return window; }

      GLFWPOMPEII_API
      vk::Extent2D swapchainImageSize( void ) const
      {
        int w, h;
        glfwGetWindowSize( window, &w, &h );
        return vk::Extent2D( w, h );
      }

      GLFWPOMPEII_API
      vk::SampleCountFlagBits sampleCountFlagBits( void ) const
      {
        return sampleCount;
      }
      GLFWPOMPEII_API
      void setSampleCountFlagBits( int sampleCount );
      GLFWPOMPEII_API
      std::vector< int > supportedSampleCounts( void );

      GLFWPOMPEII_API
      std::shared_ptr<PhysicalDevice> physicalDevice( void ) const
      {
        return _physicalDevice;
      }
      GLFWPOMPEII_API
      const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const
      {
        return physicalDevice( )->getDeviceProperties( );
      }
      GLFWPOMPEII_API
      std::shared_ptr<Device> device( void ) const
      {
        return _device;
      }
      GLFWPOMPEII_API
      std::shared_ptr<Queue> gfxQueue( void ) const
      {
        return _gfxQueue;
      }
      GLFWPOMPEII_API
      std::shared_ptr<CommandPool> gfxCommandPool( void ) const
      {
        return _cmdPool;
      }
      GLFWPOMPEII_API
      vk::Format colorFormat( void ) const
      {
        return _surfaceFormat.format;
      }
      GLFWPOMPEII_API
        vk::ColorSpaceKHR colorSpace( void ) const
      {
        return _surfaceFormat.colorSpace;
      }
      GLFWPOMPEII_API
      vk::Format depthStencilFormat( void ) const
      {
        return _dsFormat;
      }
      GLFWPOMPEII_API
      uint32_t currentIndex( void ) const
      {
        return imageIdx;
      }
      GLFWPOMPEII_API
      std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const
      {
        return cmds.at( imageIdx );
      }
      GLFWPOMPEII_API
      std::shared_ptr< RenderPass > renderPass( void ) const
      {
        return _renderPass;
      }
      GLFWPOMPEII_API
      std::vector<std::shared_ptr< Framebuffer > > framebuffers( void ) const
      {
        return _framebuffers;
      }
      GLFWPOMPEII_API
      std::shared_ptr< Framebuffer > framebuffer( void ) const
      {
        return _framebuffers.at( imageIdx );
      }
      GLFWPOMPEII_API
      std::shared_ptr< Semaphore > currentSemaphore( void ) const
      {
        return _swapchain->getPresentCompleteSemaphores( ).at( imageIdx );
      }

      GLFWPOMPEII_API
      virtual void keyEvent( int /*key*/, int /*act*/ ) { };
      GLFWPOMPEII_API
      virtual void mouseEvent( double /*xPos*/, double /*yPos*/ ) { }
      GLFWPOMPEII_API
      virtual void scrollEvent( double /*xOff*/, double /*yOff*/ ) { }
      //GLFWPOMPEII_API
      //void mouseEvent( int key, int act ) { };
    private:
      bool checkDeviceLost( vk::Result err );
      void addReadback( void );
      void finishBlockingReadback( void );

      void getSurfaceFormats( void );
      void createQueues( void );

      void beginFrame( void );
      void endFrame( std::shared_ptr<Semaphore> sem );
    public:
      GLFWPOMPEII_API
      void show( void );

    protected:
      std::shared_ptr< Swapchain > _swapchain;
      std::shared_ptr< RenderPass > _renderPass;
      std::vector<std::shared_ptr<Framebuffer>> _framebuffers;
      std::shared_ptr< Image > _depthImage;
      std::shared_ptr< ImageView > _depthView;

      void setupDepthStencilTarget( void );
    };
  }
}