#ifndef __VKLAVA_NONCOPYABLE__
#define __VKLAVA_NONCOPYABLE__

namespace vklava
{
  class NonCopyable
  {
  public:
    NonCopyable( ) = default;
    NonCopyable( NonCopyable const& ) = delete;
    NonCopyable &operator=( NonCopyable const& ) = delete;
  };
}

#endif /* __VKLAVA_NONCOPYABLE__ */