#ifndef __LAVA_SURFACE__
#define __LAVA_SURFACE__

#include "includes.hpp"
#include "noncopyable.hpp"

namespace lava
{
  class Instance;
  class Surface : private NonCopyable<Surface>
  {
  public:
    Surface( const std::shared_ptr< Instance >& instance, const vk::SurfaceKHR& surface );
    ~Surface( );

    inline operator vk::SurfaceKHR( ) const
    {
      return _surface;
    }
  private:
    std::shared_ptr<Instance> _instance;
    vk::SurfaceKHR _surface;
  };
}

#endif /* __LAVA_SURFACE__ */