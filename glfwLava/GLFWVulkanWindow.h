/**
 * Copyright (c) 2017 - 2018, Lava
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

#include <lava/lava.h>
#include <glfwLava/api.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace lava
{
  namespace glfw
  {
    class VulkanWindowRenderer
    {
    public:
      GLFWLAVA_API
      virtual ~VulkanWindowRenderer( void ) { }

      /**
      * Method called when creating renderer's resources
      */
      GLFWLAVA_API
      virtual void initResources( void ) { }
      GLFWLAVA_API
      virtual void initSwapChainResources( void ) { }
      GLFWLAVA_API
      virtual void releaseSwapChainResources( void ) { }
      /**
      * Method called when renderer's resources must be released
      */
      GLFWLAVA_API
      virtual void releaseResources( void ) { }
      /**
      * Method called when the draw calls for the next frame are to be added
      *   to the command buffer
      */
      GLFWLAVA_API
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
      GLFWLAVA_API
      virtual void setupRenderPass( void );
      GLFWLAVA_API
      virtual void setupFramebuffer( void );
	  GLFWLAVA_API
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
      GLFWLAVA_API
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
      GLFWLAVA_API
      virtual VulkanWindowRenderer* createRenderer( void );
    public:
      GLFWLAVA_API
      void requestUpdate( void )
      {
        glfwPollEvents( );
      }

      //GLFWLAVA_API
      //virtual void setVkInstance( const vk::Instance& instance );
      GLFWLAVA_API
      explicit VulkanWindow( int width, int height, 
        const std::string& title, bool enableLayers );
      GLFWLAVA_API
      VulkanWindow( const VulkanWindow& ) = delete;
      GLFWLAVA_API
      VulkanWindow( VulkanWindow&& ) = delete;
      GLFWLAVA_API
      virtual ~VulkanWindow( void );

      GLFWLAVA_API
      VulkanWindow& operator=( const VulkanWindow& ) = delete;
      GLFWLAVA_API
      VulkanWindow& operator=( VulkanWindow&& ) = delete;

      GLFWLAVA_API
      const std::shared_ptr< PipelineCache > pipelineCache( void )
      {
        return _pipelineCache;
      }
    public:
      GLFWLAVA_API
      void frameReady( std::shared_ptr<Semaphore> sem = nullptr );

     // GLFWLAVA_API
      //QImage grab( void );

      GLFWLAVA_API
      bool supportGrab( void ) const
      {
        return _swapchain ? _swapchain->swapchainSupportsReadBack( ) : false;
      }

      GLFWLAVA_API
      GLFWwindow* getWindow( void ) const { return window; }

      GLFWLAVA_API
      vk::Extent2D swapchainImageSize( void ) const
      {
        int w, h;
        glfwGetWindowSize( window, &w, &h );
        return vk::Extent2D( w, h );
      }

      GLFWLAVA_API
      vk::SampleCountFlagBits sampleCountFlagBits( void ) const
      {
        return sampleCount;
      }
      GLFWLAVA_API
      void setSampleCountFlagBits( int sampleCount );
      GLFWLAVA_API
      std::vector< int > supportedSampleCounts( void );

      GLFWLAVA_API
      std::shared_ptr<PhysicalDevice> physicalDevice( void ) const
      {
        return _physicalDevice;
      }
      GLFWLAVA_API
      const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const
      {
        return physicalDevice( )->getDeviceProperties( );
      }
      GLFWLAVA_API
      std::shared_ptr<Device> device( void ) const
      {
        return _device;
      }
      GLFWLAVA_API
      std::shared_ptr<Queue> gfxQueue( void ) const
      {
        return _gfxQueue;
      }
      GLFWLAVA_API
      std::shared_ptr<CommandPool> gfxCommandPool( void ) const
      {
        return _cmdPool;
      }
      GLFWLAVA_API
      vk::Format colorFormat( void ) const
      {
        return _surfaceFormat.format;
      }
      GLFWLAVA_API
        vk::ColorSpaceKHR colorSpace( void ) const
      {
        return _surfaceFormat.colorSpace;
      }
      GLFWLAVA_API
      vk::Format depthStencilFormat( void ) const
      {
        return _dsFormat;
      }
      GLFWLAVA_API
      uint32_t currentIndex( void ) const
      {
        return imageIdx;
      }
      GLFWLAVA_API
      std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const
      {
        return cmds.at( imageIdx );
      }
      GLFWLAVA_API
      std::shared_ptr< RenderPass > renderPass( void ) const
      {
        return _renderPass;
      }
      GLFWLAVA_API
      std::vector<std::shared_ptr< Framebuffer > > framebuffers( void ) const
      {
        return _framebuffers;
      }
      GLFWLAVA_API
      std::shared_ptr< Framebuffer > framebuffer( void ) const
      {
        return _framebuffers.at( imageIdx );
      }
      GLFWLAVA_API
      std::shared_ptr< Semaphore > currentSemaphore( void ) const
      {
        return _swapchain->getPresentCompleteSemaphores( ).at( imageIdx );
      }

      GLFWLAVA_API
      virtual void keyEvent( int /*key*/, int /*act*/ ) { };
      GLFWLAVA_API
      virtual void mouseEvent( double /*xPos*/, double /*yPos*/ ) { }
      GLFWLAVA_API
      virtual void scrollEvent( double /*xOff*/, double /*yOff*/ ) { }
      //GLFWLAVA_API
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
      GLFWLAVA_API
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