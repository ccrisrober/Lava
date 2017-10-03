#include "Surface.h"

#include "Instance.h"

namespace lava
{
  Surface::Surface( const std::shared_ptr< Instance >& instance, const vk::SurfaceKHR& surface )
    : _instance( instance )
    , _surface( surface )
  {
  }
  Surface::~Surface( void )
  {
    destroy( );
  }
  void Surface::destroy( void )
  {
    if ( _surface )
    {
      vk::Instance( *_instance ).destroySurfaceKHR( _surface );
      //_surface = VK_NULL_HANDLE;
    }
  }
}