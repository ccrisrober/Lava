#ifndef __LAVA_NONCOPYABLE__
#define __LAVA_NONCOPYABLE__

namespace lava
{
  template< class T >
  class NonCopyable
  {
  public:
    NonCopyable( void ) = default;
    NonCopyable( const NonCopyable& ) = delete;
    NonCopyable &operator=( const NonCopyable& ) = delete;
  };
}

#endif /* __LAVA_NONCOPYABLE__ */