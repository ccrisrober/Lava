#ifndef __QTPOMPEII_VULKANWINDOW__
#define __QTPOMPEII_VULKANWINDOW__

#include <pompeii/pompeii.h>
#include <qtLava/api.h>

#include <QtGui>
#include <QWindow>
#include <QVulkanInstance>

namespace pompeii
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
		QTPOMPEII_API
		virtual ~QVulkanWindowRenderer( void ) { }

		/**
		* Method called when creating renderer's resources
		*/
		QTPOMPEII_API
		virtual void initResources( void ) { }
		QTPOMPEII_API
		virtual void initSwapChainResources( void ) { }
		QTPOMPEII_API
		virtual void releaseSwapChainResources( void ) { }
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
	class QVulkanWindow: public QWindow
	{
	public:
    QTPOMPEII_API
    QVulkanWindow( QWindow* parent = 0 );

    QTPOMPEII_API
    QVulkanWindow( const QVulkanWindow& ) = delete;
    QTPOMPEII_API
    QVulkanWindow( QVulkanWindow&& ) = delete;

    QTPOMPEII_API
    QVulkanWindow& operator=( const QVulkanWindow& ) = delete;
    QTPOMPEII_API
    QVulkanWindow& operator=( QVulkanWindow&& ) = delete;

    QTPOMPEII_API
    void setVulkanInstance( const std::shared_ptr< Instance > instance );

    QTPOMPEII_API
    void resizeEvent( QResizeEvent* ) override;
    QTPOMPEII_API
    void exposeEvent( QExposeEvent* ) override;
    QTPOMPEII_API
    bool event( QEvent* event ) override;
  private:
    bool surfaceDestroyed, initialized, continuousAnimation;

    VkSurfaceKHR surface;

    bool vsync;
    bool resized;

    std::shared_ptr< pompeii::PhysicalDevice > physicalDevice;
    std::shared_ptr< pompeii::Instance > _instance;

    Swapchain2 swapchain;
  public:
    QTPOMPEII_API
    void setContinuousRendering( bool enabled ) { continuousAnimation = enabled; }

    QTPOMPEII_API
    void init( void );
    QTPOMPEII_API
    void beginFrame( void );
    QTPOMPEII_API
    void endFrame( void );
    QTPOMPEII_API
    void frameReady( void );
	};
}

#endif /* __QTPOMPEII_VULKANWINDOW__ */