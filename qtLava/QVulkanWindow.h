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

#ifndef __QTLAVA_VULKANWINDOW__
#define __QTLAVA_VULKANWINDOW__

#include <lava/lava.h>
#include <qtLava/api.h>

#include <QWindow>
#include <QtGui/QVulkanInstance>

//#include <algorithm>

namespace lava
{
  namespace qt
  {
    QTLAVA_API
    std::set<std::string> GetSurfaceExtensionsForPlatform( void );

    class VulkanWindowRenderer
    {
    public:
      QTLAVA_API
      virtual ~VulkanWindowRenderer( void ) { }

      /**
      * Method called when creating renderer's resources
      */
      QTLAVA_API
      virtual void initResources( void ) { }
      QTLAVA_API
      virtual void initSwapchainResources( void ) { }
      QTLAVA_API
      virtual void releaseSwapchainResources( void ) { }
      /**
      * Method called when renderer's resources must be released
      */
      QTLAVA_API
      virtual void releaseResources( void ) { }
      /**
      * Method called when the draw calls for the next frame are to be added
      *   to the command buffer
      */
      QTLAVA_API
      virtual void nextFrame( void ) = 0;

      //virtual void physicalDeviceLost( void );
      //virtual void logicalDeviceLost( void );
    };

    class VulkanWindow : public QWindow
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
        QTLAVA_API
        Engine( const CreateInfo &info );
        QTLAVA_API
        ~Engine( void );
        QTLAVA_API
        const vk::Instance& GetVkInstance( void ) const { return instance; }
      private:
        void createInstance( void );

        const CreateInfo info;

        vk::Instance instance;
      };
    protected:
      QTLAVA_API
      virtual void setupRenderPass( void );
      QTLAVA_API
      virtual void setupFramebuffer( void );
    protected:
      QTLAVA_API
      virtual bool setupPipelineCache( void );
    private:
      void initVulkan( void );
      void cleanupVulkan( void );
      void recreateSwapchain( void );
      bool _initialized;

      bool _continuousAnimation = false;
      
      std::shared_ptr< lava::Instance > _instance = nullptr;
      std::shared_ptr< lava::PhysicalDevice > _physicalDevice = nullptr;
      std::shared_ptr< lava::Device > _device = nullptr;
      
      std::shared_ptr< lava::Surface > _surface;
      //vk::SurfaceKHR _surface = nullptr;


      vk::SurfaceFormatKHR _surfaceFormat;
      vk::Format _dsFormat;

      uint32_t imageIdx;

      std::vector<std::string > _requestedDeviceExts;
      uint32_t _gfxQueueFamilyIdx;
      uint32_t _presQueueFamilyIdx;
      std::shared_ptr< lava::Queue > _gfxQueue;
      std::shared_ptr< lava::Queue > _presQueue;

      std::vector<std::shared_ptr< lava::CommandBuffer > > cmds;

      std::shared_ptr< lava::CommandPool > _cmdPool;

      bool _framePending = false;
      bool _frameGrabbing = false;
      QImage _frameGrabTargetImage;
      std::shared_ptr< lava::Image > frameGrabImage = nullptr;
    protected:
      std::shared_ptr< PipelineCache > _pipelineCache;
    private:
      vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

      std::shared_ptr< lava::Semaphore > _renderComplete = nullptr;
      VulkanWindowRenderer* renderer = nullptr;
    protected:
      virtual VulkanWindowRenderer* createRenderer( void );
    public:
      QTLAVA_API
      virtual void setVkInstance( const vk::Instance& instance );
      QTLAVA_API
      explicit VulkanWindow( QWindow* parent = 0 );
      QTLAVA_API
      VulkanWindow( const VulkanWindow& ) = delete;
      QTLAVA_API
      VulkanWindow( VulkanWindow&& ) = delete;
      QTLAVA_API
      virtual ~VulkanWindow( void );

      QTLAVA_API
      VulkanWindow& operator=( const VulkanWindow& ) = delete;
      QTLAVA_API
      VulkanWindow& operator=( VulkanWindow&& ) = delete;

      QTLAVA_API
      const std::shared_ptr< PipelineCache > pipelineCache( void )
      {
        return _pipelineCache;
      }
    public:
      QTLAVA_API
      void frameReady( void );

      QTLAVA_API
      QImage grab( void );

      QTLAVA_API
      bool supportGrab( void ) const
      {
        return _swapchain ? _swapchain->swapchainSupportsReadBack( ) : false;
      }

      QTLAVA_API
      vk::Extent2D swapchainImageSize( void ) const
      {
        QSize _size = size( );
        return vk::Extent2D(
          _size.width( ),
          _size.height( )
        );
      }

      QTLAVA_API
      vk::SampleCountFlagBits sampleCountFlagBits( void ) const
      {
        return sampleCount;
      }
      QTLAVA_API
      void setSampleCountFlagBits( int sampleCount );
      QTLAVA_API
      std::vector< int > supportedSampleCounts( void );

      const bool isContinuosRendering( void ) const
      {
        return _continuousAnimation;
      }
      QTLAVA_API
      void setContinuousRendering( bool enabled )
      {
        _continuousAnimation = enabled;
        if ( _initialized )
        {
          requestUpdate( );
        }
      }

      QTLAVA_API
      std::shared_ptr<PhysicalDevice> physicalDevice( void ) const
      {
        return _physicalDevice;
      }
      QTLAVA_API
      const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const
      {
        return physicalDevice( )->getDeviceProperties( );
      }
      QTLAVA_API
      std::shared_ptr<Device> device( void ) const
      {
        return _device;
      }
      QTLAVA_API
      std::shared_ptr<Queue> gfxQueue( void ) const
      {
        return _gfxQueue;
      }
      QTLAVA_API
      std::shared_ptr<CommandPool> gfxCommandPool( void ) const
      {
        return _cmdPool;
      }
      QTLAVA_API
      vk::Format colorFormat( void ) const
      {
        return _surfaceFormat.format;
      }
      QTLAVA_API
        vk::ColorSpaceKHR colorSpace( void ) const
      {
        return _surfaceFormat.colorSpace;
      }
      QTLAVA_API
      vk::Format depthStencilFormat( void ) const
      {
        return _dsFormat;
      }
      QTLAVA_API
      std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const
      {
        return cmds.at( imageIdx );
      }
      QTLAVA_API
      std::shared_ptr< RenderPass > renderPass( void ) const
      {
        return _renderPass;
      }
      QTLAVA_API
      std::shared_ptr< Framebuffer > framebuffer( void ) const
      {
        return _framebuffers.at( imageIdx );
      }
    protected:
      QTLAVA_API
      bool event( QEvent* ev ) override;
      QTLAVA_API
      void exposeEvent( QExposeEvent* ev ) override;
      QTLAVA_API
      void resizeEvent( QResizeEvent* ev ) override;

    private:
      bool checkDeviceLost( vk::Result err );
      void addReadback( void );
      void finishBlockingReadback( void );

      void getSurfaceFormats( void );
      void createQueues( void );

      QVulkanInstance inst;

      //QTLAVA_API
      void beginFrame( void );
      //QTLAVA_API
      void endFrame( void );

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

#endif /* __QTLAVA_VULKANWINDOW__ */