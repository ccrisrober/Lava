#ifndef __LAVA_MEMORY_UTILS__
#define __LAVA_MEMORY_UTILS__

namespace lava
{
  template<typename T, typename... Args>
  static std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}

#endif /* __LAVA_MEMORY_UTILS__ */
