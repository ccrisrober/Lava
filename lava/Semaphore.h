#ifndef __LAVA_SEMAPHORE__
#define __LAVA_SEMAPHORE__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace lava
{
  class Device;
  class Semaphore
    : public VulkanResource
    , private NonCopyable< Semaphore >
  {
  public:
    LAVA_API
    Semaphore( const DeviceRef& device );
    Semaphore( const Semaphore& ) = delete;

    Semaphore& operator=( const Semaphore& ) = delete;
    virtual ~Semaphore( void );

    LAVA_API
    inline operator vk::Semaphore( void )
    {
      return _semaphore;
    }

  protected:
    vk::Semaphore _semaphore;
  };
}

#endif /* __LAVA_SEMAPHORE__ */