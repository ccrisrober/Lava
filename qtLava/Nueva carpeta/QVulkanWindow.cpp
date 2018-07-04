#include "QVulkanWindow.h"

namespace lava
{
  QVulkanWindow::QVulkanWindow( QWindow * parent )
    : QWindow( parent )
  {
    setSurfaceType( QSurface::SurfaceType::VulkanSurface );
  }
  void QVulkanWindow::setVulkanInstance( const std::shared_ptr<Instance> instance )
  {
    _instance = instance;

    VkInstance vki = static_cast< VkInstance >( static_cast< vk::Instance >( *instance ) );
    auto _qInstance = new QVulkanInstance( );
    _qInstance->setVkInstance( vki );
    if ( _qInstance->create( ) )
    {
      QWindow::setVulkanInstance( _qInstance );
    }
  }
  void QVulkanWindow::resizeEvent( QResizeEvent* event )
  {
    if ( swapchain.isReady( ) )
    {
      resized = true;
    }
    QWindow::resizeEvent( event );
  }

  void QVulkanWindow::exposeEvent( QExposeEvent* event )
  {
    if ( isExposed( ) )
    {
      if ( !initialized )
      {
        init( );
      }
      else if ( !swapchain.isReady( ) )
      {
        if ( surfaceDestroyed )
        {
          surface = vulkanInstance( )->surfaceForWindow( this );
          swapchain.recreate( surface );
          surfaceDestroyed = false;
        }
        else
        {
          swapchain.recreate( );
        }
      }
      beginFrame( );
    }
    else
    {
      swapchain.destroy( );
    }
    QWindow::exposeEvent( event );
  }

  bool QVulkanWindow::event( QEvent* event )
  {
    switch ( event->type( ) )
    {
    case QEvent::UpdateRequest:
      beginFrame( );
      break;
    case QEvent::PlatformSurface:
      if ( static_cast<QPlatformSurfaceEvent*>( event )->surfaceEventType( )
        == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed )
      {
        swapchain.destroy( );
        surfaceDestroyed = true;
      }
      break;
    default:
      break;
    }

    return QWindow::event( event );
  }
  void QVulkanWindow::init( void )
  {
    if ( !_instance )
    {
      throw "Instance don't exist";
    }

    surface = vulkanInstance( )->surfaceForWindow( this );

    // Find a physical device with presentation support
    assert( _instance->getPhysicalDeviceCount( ) != 0 );
    physicalDevice = _instance->getPhysicalDevice( 0 );

    if ( !physicalDevice )
    {
      LAVA_RUNTIME_ERROR( "Failed to find a device with presentation support" );
    }

    initialized = true;
  }
  void QVulkanWindow::beginFrame( void )
  {
    if ( !swapchain.isReady( ) ) return;

    if ( resized )
    {
      swapchain.resize( static_cast<uint32_t>( width( ) ),
        static_cast<uint32_t>( height( ) ) );
      resized = false;
    }
  }
  void QVulkanWindow::endFrame( void )
  {


    if ( continuousAnimation )
    {
      requestUpdate( );
    }
  }
}