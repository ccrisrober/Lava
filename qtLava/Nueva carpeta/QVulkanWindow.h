#ifndef __QTLAVA_VULKANWINDOW__
#define __QTLAVA_VULKANWINDOW__

#include <lava/lava.h>
#include <qtLava/api.h>

#include <QtGui>
#include <QWindow>
#include <QVulkanInstance>

namespace lava
{
  class Swapchain2
  {
  public:
    ~Swapchain2( void )
    {
      destroy( );
    }
    void init( std::shared_ptr<Device>& device, 
      VkSurfaceKHR surface, uint32_t width, uint32_t height,
      bool vsync )
    {
      this->device = device;
      create( );
    }
    bool isReady( void ) const// override
    {
      return handle != VK_NULL_HANDLE;
    }

    void resize( uint32_t width, uint32_t height ) // override;
    {

    }
    void recreate( VkSurfaceKHR newSurface = VK_NULL_HANDLE )
    {
      if ( newSurface != VK_NULL_HANDLE )
      {
        destroy( );
        surface = newSurface;
      }
      create( );
      //resizeEvent( )
    }
    void create( void )
    {

    }
    void destroy( void )
    {
      if ( !isReady( ) )
      {
        return;
      }

      handle = VK_NULL_HANDLE;
    }
  protected:
    std::shared_ptr<Device> device;
    VkSurfaceKHR surface;
    vk::SwapchainKHR handle;
  };


	class QVulkanWindowRenderer
	{
	public:
		QTLAVA_API
		virtual ~QVulkanWindowRenderer( void ) { }

		/**
		* Method called when creating renderer's resources
		*/
		QTLAVA_API
		virtual void initResources( void ) { }
		QTLAVA_API
		virtual void initSwapChainResources( void ) { }
		QTLAVA_API
		virtual void releaseSwapChainResources( void ) { }
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
	class QVulkanWindow: public QWindow
	{
	public:
    QTLAVA_API
    QVulkanWindow( QWindow* parent = 0 );

    QTLAVA_API
    QVulkanWindow( const QVulkanWindow& ) = delete;
    QTLAVA_API
    QVulkanWindow( QVulkanWindow&& ) = delete;

    QTLAVA_API
    QVulkanWindow& operator=( const QVulkanWindow& ) = delete;
    QTLAVA_API
    QVulkanWindow& operator=( QVulkanWindow&& ) = delete;

    QTLAVA_API
    void setVulkanInstance( const std::shared_ptr< Instance > instance );

    QTLAVA_API
    void resizeEvent( QResizeEvent* ) override;
    QTLAVA_API
    void exposeEvent( QExposeEvent* ) override;
    QTLAVA_API
    bool event( QEvent* event ) override;
  private:
    bool surfaceDestroyed, initialized, continuousAnimation;

    VkSurfaceKHR surface;

    bool vsync;
    bool resized;

    std::shared_ptr< lava::PhysicalDevice > physicalDevice;
    std::shared_ptr< lava::Instance > _instance;

    Swapchain2 swapchain;
  public:
    QTLAVA_API
    void setContinuousRendering( bool enabled ) { continuousAnimation = enabled; }

    QTLAVA_API
    void init( void );
    QTLAVA_API
    void beginFrame( void );
    QTLAVA_API
    void endFrame( void );
    QTLAVA_API
    void frameReady( void );
	};
}

#endif /* __QTLAVA_VULKANWINDOW__ */