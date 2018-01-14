#ifndef __LAVA_EVENT__
#define __LAVA_EVENT__

#include "includes.hpp"

#include "VulkanResource.h"

#include <lava/api.h>

namespace lava
{
  class Device;

  class Event : public VulkanResource, 
    public std::enable_shared_from_this< Event >
  {
  public:
    LAVA_API
    Event( const std::shared_ptr<Device>& device );

    Event( const Event& rhs ) = delete;
    Event& operator=( const Event& rhs ) = delete;
    
    LAVA_API
    virtual ~Event( void );

    LAVA_API
    bool isSignaled( void ) const;
    LAVA_API
    void reset( void );
    LAVA_API
    void set( void );

    LAVA_API
    inline operator vk::Event( void ) const
    {
      return _event;
    }

  private:
    vk::Event _event;
  };
}

#endif /* __LAVA_EVENT__ */