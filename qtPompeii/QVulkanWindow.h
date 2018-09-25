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

#ifndef __QTPOMPEII_VULKANWINDOW__
#define __QTPOMPEII_VULKANWINDOW__

#include <pompeii/pompeii.h>
#include <qtPompeii/api.h>

#include <QWindow>
#include <QtGui/QVulkanInstance>

//#include <algorithm>

namespace pompeii
{
  namespace qt
  {
    QTPOMPEII_API
    std::set<std::string> GetSurfaceExtensionsForPlatform( void );

    class VulkanWindowRenderer
    {
    public:
      QTPOMPEII_API
      virtual ~VulkanWindowRenderer( void ) { }

      /**
      * Method called when creating renderer's resources
      */
      QTPOMPEII_API
      virtual void initResources( void ) { }
      QTPOMPEII_API
      virtual void initSwapchainResources( void ) { }
      QTPOMPEII_API
      virtual void releaseSwapchainResources( void ) { }
      /**
      * Method called when renderer's resources must be released
      */
      QTPOMPEII_API
      virtual void releaseResources( void ) { }
      /**
      * Method called when the draw calls for the next frame are to be added
      *   to the command buffer
      */
      QTPOMPEII_API
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
        QTPOMPEII_API
        Engine( const CreateInfo &info );
        QTPOMPEII_API
        ~Engine( void );
        QTPOMPEII_API
        const vk::Instance& GetVkInstance( void ) const { return instance; }
      private:
        void createInstance( void );

        const CreateInfo info;

        vk::Instance instance;
      };
    protected:
      QTPOMPEII_API
      virtual void setupRenderPass( void );
      QTPOMPEII_API
      virtual void setupFramebuffer( void );
    protected:
      QTPOMPEII_API
      virtual bool setupPipelineCache( void );
    private:
      void initVulkan( void );
      void cleanupVulkan( void );
      void recreateSwapchain( void );
      bool _initialized;

      bool _continuousAnimation = false;
      
      std::shared_ptr< pompeii::Instance > _instance = nullptr;
      std::shared_ptr< pompeii::PhysicalDevice > _physicalDevice = nullptr;
      std::shared_ptr< pompeii::Device > _device = nullptr;
      
      std::shared_ptr< pompeii::Surface > _surface;
      //vk::SurfaceKHR _surface = nullptr;


      vk::SurfaceFormatKHR _surfaceFormat;
      vk::Format _dsFormat;

      uint32_t imageIdx;

      std::vector<std::string > _requestedDeviceExts;
      uint32_t _gfxQueueFamilyIdx;
      uint32_t _presQueueFamilyIdx;
      std::shared_ptr< pompeii::Queue > _gfxQueue;
      std::shared_ptr< pompeii::Queue > _presQueue;

      std::vector<std::shared_ptr< pompeii::CommandBuffer > > cmds;

      std::shared_ptr< pompeii::CommandPool > _cmdPool;

      bool _framePending = false;
      bool _frameGrabbing = false;
      QImage _frameGrabTargetImage;
      std::shared_ptr< pompeii::Image > frameGrabImage = nullptr;
    protected:
      std::shared_ptr< PipelineCache > _pipelineCache;
    private:
      vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

      std::shared_ptr< pompeii::Semaphore > _renderComplete = nullptr;
      VulkanWindowRenderer* renderer = nullptr;
    protected:
      virtual VulkanWindowRenderer* createRenderer( void );
    public:
      QTPOMPEII_API
      virtual void setVkInstance( const vk::Instance& instance );
      QTPOMPEII_API
      explicit VulkanWindow( QWindow* parent = 0 );
      QTPOMPEII_API
      VulkanWindow( const VulkanWindow& ) = delete;
      QTPOMPEII_API
      VulkanWindow( VulkanWindow&& ) = delete;
      QTPOMPEII_API
      virtual ~VulkanWindow( void );

      QTPOMPEII_API
      VulkanWindow& operator=( const VulkanWindow& ) = delete;
      QTPOMPEII_API
      VulkanWindow& operator=( VulkanWindow&& ) = delete;

      QTPOMPEII_API
      const std::shared_ptr< PipelineCache > pipelineCache( void )
      {
        return _pipelineCache;
      }
    public:
      QTPOMPEII_API
      void frameReady( void );

      QTPOMPEII_API
      QImage grab( void );

      QTPOMPEII_API
      bool supportGrab( void ) const
      {
        return _swapchain ? _swapchain->swapchainSupportsReadBack( ) : false;
      }

      QTPOMPEII_API
      vk::Extent2D swapchainImageSize( void ) const
      {
        QSize _size = size( );
        return vk::Extent2D(
          _size.width( ),
          _size.height( )
        );
      }

      QTPOMPEII_API
      vk::SampleCountFlagBits sampleCountFlagBits( void ) const
      {
        return sampleCount;
      }
      QTPOMPEII_API
      void setSampleCountFlagBits( int sampleCount );
      QTPOMPEII_API
      std::vector< int > supportedSampleCounts( void );

      const bool isContinuosRendering( void ) const
      {
        return _continuousAnimation;
      }
      QTPOMPEII_API
      void setContinuousRendering( bool enabled )
      {
        _continuousAnimation = enabled;
        if ( _initialized )
        {
          requestUpdate( );
        }
      }

      QTPOMPEII_API
      std::shared_ptr<PhysicalDevice> physicalDevice( void ) const
      {
        return _physicalDevice;
      }
      QTPOMPEII_API
      const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const
      {
        return physicalDevice( )->getDeviceProperties( );
      }
      QTPOMPEII_API
      std::shared_ptr<Device> device( void ) const
      {
        return _device;
      }
      QTPOMPEII_API
      std::shared_ptr<Queue> gfxQueue( void ) const
      {
        return _gfxQueue;
      }
      QTPOMPEII_API
      std::shared_ptr<CommandPool> gfxCommandPool( void ) const
      {
        return _cmdPool;
      }
      QTPOMPEII_API
      vk::Format colorFormat( void ) const
      {
        return _surfaceFormat.format;
      }
      QTPOMPEII_API
        vk::ColorSpaceKHR colorSpace( void ) const
      {
        return _surfaceFormat.colorSpace;
      }
      QTPOMPEII_API
      vk::Format depthStencilFormat( void ) const
      {
        return _dsFormat;
      }
      QTPOMPEII_API
      std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const
      {
        return cmds.at( imageIdx );
      }
      QTPOMPEII_API
      std::shared_ptr< RenderPass > renderPass( void ) const
      {
        return _renderPass;
      }
      QTPOMPEII_API
      std::shared_ptr< Framebuffer > framebuffer( void ) const
      {
        return _framebuffers.at( imageIdx );
      }
    protected:
      QTPOMPEII_API
      bool event( QEvent* ev ) override;
      QTPOMPEII_API
      void exposeEvent( QExposeEvent* ev ) override;
      QTPOMPEII_API
      void resizeEvent( QResizeEvent* ev ) override;

    private:
      bool checkDeviceLost( vk::Result err );
      void addReadback( void );
      void finishBlockingReadback( void );

      void getSurfaceFormats( void );
      void createQueues( void );

      QVulkanInstance inst;

      //QTPOMPEII_API
      void beginFrame( void );
      //QTPOMPEII_API
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

#endif /* __QTPOMPEII_VULKANWINDOW__ */