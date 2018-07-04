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

#ifndef __GLFWLAVA_VULKAN_RENDERER__
#define __GLFWLAVA_VULKAN_RENDERER__

#include <lava/lava.h>
#include <glfwLava/api.h>

#include "DefaultFramebuffer.h"
#include "Window.h"

namespace lava
{
  class VulkanWindowRenderer
  {
  public:
  GLFWLAVA_API
  virtual ~VulkanWindowRenderer( void );

  /**
  * Method called when creating renderer's resources
  */
  GLFWLAVA_API
  virtual void initResources( void );
  GLFWLAVA_API
  virtual void initSwapChainResources( void );
  GLFWLAVA_API
  virtual void releaseSwapChainResources( void );
  /**
  * Method called when renderer's resources must be released
  */
  GLFWLAVA_API
  virtual void releaseResources( void );
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
  public:
  GLFWLAVA_API
  VulkanWindow( void)
  {
    _initialized = false;
    _swapChainImageSize = vk::Offset2D( 500, 500 );
  }
  GLFWLAVA_API
  virtual ~VulkanWindow( void );

  GLFWLAVA_API
  virtual void getEnabledFeatures( vk::PhysicalDeviceFeatures& ) { };

  GLFWLAVA_API
  VulkanWindow( const VulkanWindow& ) = delete;
  GLFWLAVA_API
  VulkanWindow( VulkanWindow&& ) = delete;

  GLFWLAVA_API
  VulkanWindow& operator=( const VulkanWindow& ) = delete;
  GLFWLAVA_API
  VulkanWindow& operator=( VulkanWindow&& ) = delete;

  GLFWLAVA_API
  void setVulkanInstance( const std::shared_ptr< Instance > instance );

  GLFWLAVA_API
  bool supportsGrab( void ) const;

  GLFWLAVA_API
  void init( void );
  GLFWLAVA_API
  void reset( void );

  GLFWLAVA_API
  void beginFrame( void );
  GLFWLAVA_API
  void endFrame( std::shared_ptr<Semaphore> sem );
  GLFWLAVA_API
  bool checkDeviceLost( vk::Result res );

  GLFWLAVA_API
  std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const;

  GLFWLAVA_API
  RenderAPICapabilities caps( void ) const;

  GLFWLAVA_API
  void setWindowTitle( const char*& name );
  GLFWLAVA_API
  void setWindowTitle( const std::string& name );

  GLFWLAVA_API
  vk::Extent2D getExtent( void ) const
  {
    return _defaultFramebuffer->getExtent( );
  }

  GLFWLAVA_API
  std::shared_ptr< PhysicalDevice > physicalDevice( void ) const;
  GLFWLAVA_API
  const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const;
  GLFWLAVA_API
  std::shared_ptr< Device > device( void ) const;
  GLFWLAVA_API
  std::shared_ptr< Queue > gfxQueue( void ) const;
  GLFWLAVA_API
  std::shared_ptr< CommandPool > gfxCommandPool( void ) const;
  GLFWLAVA_API
  std::shared_ptr< RenderPass > defaultRenderPass( void ) const;

  GLFWLAVA_API
  vk::Format colorFormat( void ) const;
  GLFWLAVA_API
  vk::Format depthStencilFormat( void ) const;
  GLFWLAVA_API
  vk::Offset2D swapChainImageSize( void ) const;

  int currentFrame( void ) const;

  GLFWLAVA_API
  int swapChainImageCount( void ) const;
  GLFWLAVA_API
  int currentSwapChainImageIndex( void ) const;
  GLFWLAVA_API
  std::shared_ptr< Image > swapChainImage( int idx ) const;
  GLFWLAVA_API
  std::shared_ptr< ImageView > swapChainImageView( int idx ) const;
  GLFWLAVA_API
  std::shared_ptr< Image > depthStencilImage( void ) const;
  GLFWLAVA_API
  std::shared_ptr< ImageView > depthStencilImageView( void ) const;

  //GLFWLAVA_API
  //glm::mat4 clipCorrectionMatrix( void );

  GLFWLAVA_API
  virtual void resize( uint32_t width, uint32_t height ) final
  {
    //assert( ( 0 <= width ) && ( 0 <= height ) );

    _swapChainImageSize.x = width;
    _swapChainImageSize.y = height;

    resizeEvent( width, height );
  }

  GLFWLAVA_API
  virtual void resizeEvent( uint32_t w, uint32_t h );

  GLFWLAVA_API
  void show( void );

  std::shared_ptr< PipelineCache > pipelineCache;

  GLFWLAVA_API
  /*std::unique_ptr< */glfw::DefaultFramebuffer* /*>*/ defaultFramebuffer( void ) const // TODO: SO UGLY ...
  {
    return _defaultFramebuffer.get( );
  }

  GLFWLAVA_API
  std::shared_ptr< Framebuffer > currentFramebuffer( void ) const
  {
    if ( !_framePending )
    {
      throw "Attemped to call currentFramebuffer( ) without a active frame";
    }
    return _defaultFramebuffer->getFramebuffer( );
  }
  GLFWLAVA_API
  virtual VulkanWindowRenderer* createRenderer( void );

  GLFWLAVA_API
  void requestUpdate( std::shared_ptr<Semaphore> sem = nullptr );

  /**
    Reurnt current sample count.
  */
  GLFWLAVA_API
  vk::SampleCountFlagBits sampleCountFlagBits( void ) const;

  GLFWLAVA_API
  void setSampleCountFlagBits( int sampleCount );

  GLFWLAVA_API
  std::vector< int > supportedSampleCounts( void );

  GLFWLAVA_API
  int imagesCount( void ) const;

  GLFWLAVA_API
  void setDeviceExtensions( const std::vector< std::string >& extensions );

  GLFWLAVA_API
  std::shared_ptr<Instance> vulkanInstance( void )
  {
    return _instance;
  }
  protected:
  GLFWLAVA_API
  virtual bool setupRenderPass( void );

  GLFWLAVA_API
  virtual bool setupFramebuffer( void );

  GLFWLAVA_API
  virtual bool setupPipelineCache( void );

  GLFWLAVA_API
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
  std::shared_ptr< Image > _frameGrabTargetImage = nullptr;
  std::shared_ptr< Image > frameGrabImage = nullptr;

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
  private:
    void addReadback( void );
    void finishBlockingReadback( void );
  public:
    std::shared_ptr< Image > grab( void );
  };
}

#endif /* __GLFWLAVA_VULKAN_RENDERER__ */