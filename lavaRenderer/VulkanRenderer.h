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

#ifndef __LAVARENDERER_VULKAN_RENDERER__
#define __LAVARENDERER_VULKAN_RENDERER__

#include <lava/lava.h>
#include <lavaRenderer/api.h>

#include "DefaultFramebuffer.h"
#include "Window.h"

namespace lava
{
  class VulkanWindowRenderer
  {
  public:
    LAVARENDERER_API
    virtual ~VulkanWindowRenderer( void );

    /**
    * Method called when creating renderer's resources
    */
    LAVARENDERER_API
    virtual void initResources( void );
    LAVARENDERER_API
    virtual void initSwapChainResources( void );
    LAVARENDERER_API
    virtual void releaseSwapChainResources( void );
    /**
    * Method called when renderer's resources must be released
    */
    LAVARENDERER_API
    virtual void releaseResources( void );
    /**
    * Method called when the draw calls for the next frame are to be added
    *   to the command buffer
    */
    LAVARENDERER_API
    virtual void nextFrame( void ) = 0;

    //virtual void physicalDeviceLost( void );
    //virtual void logicalDeviceLost( void );
  };

  class VulkanWindow
  {
  public:
    LAVARENDERER_API
    VulkanWindow( void)
    {
      _initialized = false;
      _swapChainImageSize = vk::Offset2D( 500, 500 );
    }
    LAVARENDERER_API
    virtual ~VulkanWindow( void );

    LAVARENDERER_API
    VulkanWindow( const VulkanWindow& ) = delete;
    LAVARENDERER_API
    VulkanWindow( VulkanWindow&& ) = delete;

    LAVARENDERER_API
    VulkanWindow& operator=( const VulkanWindow& ) = delete;
    LAVARENDERER_API
    VulkanWindow& operator=( VulkanWindow&& ) = delete;

    LAVARENDERER_API
    void setVulkanInstance( const std::shared_ptr< Instance > instance );

    LAVARENDERER_API
    bool supportsGrab( void ) const;

    LAVARENDERER_API
    void init( void );
    LAVARENDERER_API
    void reset( void );

    LAVARENDERER_API
    void beginFrame( void );
    LAVARENDERER_API
    void endFrame( std::shared_ptr<Semaphore> sem );
    LAVARENDERER_API
    bool checkDeviceLost( vk::Result res );

    LAVARENDERER_API
    std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const;

    LAVARENDERER_API
    RenderAPICapabilities caps( void ) const;

    LAVARENDERER_API
    void setWindowTitle( const char*& name );
    LAVARENDERER_API
    void setWindowTitle( const std::string& name );

    LAVARENDERER_API
    vk::Extent2D getExtent( void ) const
    {
      return _defaultFramebuffer->getExtent( );
    }

    LAVARENDERER_API
    std::shared_ptr< PhysicalDevice > physicalDevice( void ) const;
    LAVARENDERER_API
    const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const;
    LAVARENDERER_API
    std::shared_ptr< Device > device( void ) const;
    LAVARENDERER_API
    std::shared_ptr< Queue > gfxQueue( void ) const;
    LAVARENDERER_API
    std::shared_ptr< CommandPool > gfxCommandPool( void ) const;
    LAVARENDERER_API
    std::shared_ptr< RenderPass > defaultRenderPass( void ) const;

    LAVARENDERER_API
    vk::Format colorFormat( void ) const;
    LAVARENDERER_API
    vk::Format depthStencilFormat( void ) const;
    LAVARENDERER_API
    vk::Offset2D swapChainImageSize( void ) const;

    int currentFrame( void ) const;

    LAVARENDERER_API
    int swapChainImageCount( void ) const;
    LAVARENDERER_API
    int currentSwapChainImageIndex( void ) const;
    LAVARENDERER_API
    std::shared_ptr< Image > swapChainImage( int idx ) const;
    LAVARENDERER_API
    std::shared_ptr< ImageView > swapChainImageView( int idx ) const;
    LAVARENDERER_API
    std::shared_ptr< Image > depthStencilImage( void ) const;
    LAVARENDERER_API
    std::shared_ptr< ImageView > depthStencilImageView( void ) const;

    //LAVARENDERER_API
    //glm::mat4 clipCorrectionMatrix( void );

    LAVARENDERER_API
    virtual void resize( uint32_t width, uint32_t height ) final
    {
      //assert( ( 0 <= width ) && ( 0 <= height ) );

      _swapChainImageSize.x = width;
      _swapChainImageSize.y = height;

      resizeEvent( width, height );
    }

    LAVARENDERER_API
    virtual void resizeEvent( uint32_t w, uint32_t h );

    LAVARENDERER_API
    void show( void );

    std::shared_ptr< PipelineCache > pipelineCache;

    LAVARENDERER_API
    /*std::unique_ptr< */glfw::DefaultFramebuffer* /*>*/ defaultFramebuffer( void ) const // TODO: SO UGLY ...
    {
      return _defaultFramebuffer.get( );
    }

    LAVARENDERER_API
    std::shared_ptr< Framebuffer > currentFramebuffer( void ) const
    {
      return _defaultFramebuffer->getFramebuffer( );
    }
    LAVARENDERER_API
    virtual VulkanWindowRenderer* createRenderer( void );

    LAVARENDERER_API
    void requestUpdate( std::shared_ptr<Semaphore> sem = nullptr );

    LAVARENDERER_API
    vk::SampleCountFlagBits sampleCountFlagBits( void ) const;

    LAVARENDERER_API
    void setSampleCountFlagBits( int sampleCount );

    LAVARENDERER_API
    std::vector< int > supportedSampleCounts( void );

    LAVARENDERER_API
    int imagesCount( void ) const;

    LAVARENDERER_API
    void setDeviceExtensions( const std::vector< std::string >& extensions );

    virtual void getEnabledFeatures( void )
    {
    }

    LAVARENDERER_API
    std::shared_ptr<Instance> vulkanInstance( void )
    {
      return _instance;
    }
  protected:
    LAVARENDERER_API
    virtual bool setupRenderPass( void );

    LAVARENDERER_API
    virtual bool setupFramebuffer( void );

    LAVARENDERER_API
    virtual bool setupPipelineCache( void );

    LAVARENDERER_API
    std::shared_ptr< Image > recordImage( void );

  private:
    void initCapabilites( void );
    void recreateSwapChain( void );

    bool _initialized;

    VulkanWindowRenderer* renderer = nullptr;
    
    std::vector<std::string > _requestedDeviceExts;

  protected:
    int numero = 0;
    std::shared_ptr< Instance > _instance;
    std::shared_ptr< PhysicalDevice > _physicalDevice;
    std::shared_ptr< Device > _device;
    std::shared_ptr< RenderPass > _renderPass;
    
    std::shared_ptr< Surface > _surface;

    std::unique_ptr< glfw::DefaultFramebuffer > _defaultFramebuffer;

    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
    

    uint32_t _gfxQueueFamilyIdx;
    uint32_t _presQueueFamilyIdx;
    std::shared_ptr< Queue > _gfxQueue;
    std::shared_ptr< Queue > _presQueue;
  public:
    std::shared_ptr< Semaphore > _renderComplete;
  protected:

    struct ImageResources
    {
      std::shared_ptr<CommandBuffer> commandBuffer;
    } imageRes[ 2 ];
  public:
    std::shared_ptr< CommandPool > _cmdPool;
    std::shared_ptr< Window > _window;
  protected:
    vk::Offset2D _swapChainImageSize;
    vk::Format _colorFormat;
    vk::ColorSpaceKHR _colorSpace;
    vk::Format _dsFormat;

    //vk::PresentModeKHR presentMode = vk::PresentMode::eFifoKHR;

    //glm::mat4 _clipCorrect;

    RenderAPICapabilities _caps;
    uint32_t _currentFrame;


    bool _framePending = false;
    bool _frameRecord = false;
    std::shared_ptr< Image > _frameRecordImage = nullptr;

    private:
      std::shared_ptr<Surface> createSurfaceKHR( GLFWwindow* window )
      {
        // Surface KHR
        VkSurfaceKHR surface;
        if ( glfwCreateWindowSurface( VkInstance( 
          static_cast< vk::Instance >( *_instance ) ), window, nullptr, 
          &surface ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to create window surface!" );
        }
        return std::make_shared<Surface>( _instance, vk::SurfaceKHR( surface ) );
      }
  };
}

#endif /* __LAVARENDERER_VULKAN_RENDERER__ */