#ifndef __LAVA_SEMAPHORE__
#define __LAVA_SEMAPHORE__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace lava
{
  class Device;
  class Semaphore : public VulkanResource, private NonCopyable<Semaphore>
  {
  public:
    Semaphore( const DeviceRef& device );
    virtual ~Semaphore( void );

    inline operator vk::Semaphore( void )
    {
      return _semaphore;
    }

  protected:
    vk::Semaphore _semaphore;
  };
}

#endif /* __LAVA_SEMAPHORE__ */