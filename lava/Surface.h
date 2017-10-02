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
    Surface( const std::shared_ptr< Instance >& instance, 
      const vk::SurfaceKHR& surface );
    Surface( const Surface& ) = delete;

    Surface& operator=( const Surface& ) = delete;
    ~Surface( void );

    inline operator vk::SurfaceKHR( void ) const
    {
      return _surface;
    }
    void destroy( void );
  private:
    std::shared_ptr<Instance> _instance;
    vk::SurfaceKHR _surface;
  };
}

#endif /* __LAVA_SURFACE__ */