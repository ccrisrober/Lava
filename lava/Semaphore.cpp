#include "Semaphore.h"

#include "Device.h"

namespace lava
{
  Semaphore::Semaphore( const DeviceRef& device )
    : VulkanResource( device )
  {
    _semaphore = vk::Device( *_device ).createSemaphore( vk::SemaphoreCreateInfo( ) );
  }
  Semaphore::~Semaphore( void )
  {
    destroy( );
  }
  void Semaphore::destroy( void )
  {
    if ( _semaphore != VK_NULL_HANDLE )
    {
      vk::Device( *_device ).destroySemaphore( _semaphore );
      _semaphore = VK_NULL_HANDLE;
    }
  }
}