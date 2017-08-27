#ifndef __VKLAVA_NONCOPYABLE__
#define __VKLAVA_NONCOPYABLE__

namespace lava
{
  template< class T >
  class NonCopyable
  {
  public:
    NonCopyable( void ) = default;
    NonCopyable(const NonCopyable& ) = delete;
    NonCopyable &operator=(const NonCopyable& ) = delete;
  };
}

#endif /* __VKLAVA_NONCOPYABLE__ */