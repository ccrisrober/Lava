#ifndef __LAVA_INCLUDES__
#define __LAVA_INCLUDES__

/*#if defined ( __ANDROID__ )
  #define LAVA_PLATFORM_ANDROID
#elif defined( __APPLE__ )
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #define LAVA_PLATFORM_IOS
  #else
    #define LAVA_PLATFORM_OSX
  #endif
#elif defined( __MINGW32__ ) || defined( WIN32 ) || defined( __WIN32__ )
  #define LAVA_PLATFORM_WINDOWS
#else
  #define LAVA_PLATFORM_LINUX
#endif

#if defined( LAVA_PLATFORM_WINDOWS ) || defined( LAVA_PLATFORM_LINUX ) ||defined( LAVA_PLATFORM_OSX )
  #define LAVA_PLATFORM_DESKTOP
#endif

#if defined( LAVA_PLATFORM_IOS ) || defined( LAVA_PLATFORM_ANDROID )
  #define LAVA_PLATFORM_MOBILE
#endif*/

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#if defined( __MINGW32__ ) || defined( WIN32 ) || defined( __WIN32__ )
  #define LAVA_PLATFORM_WINDOWS
  #define LAVA_KHR_EXT "VK_KHR_win32_surface"
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
  #define LAVA_PLATFORM_ANDROID
  #define LAVA_KHR_EXT "VK_USE_PLATFORM_ANDROID_KHR"
#elif defined(_DIRECT2DISPLAY)
  #define LAVA_KKHR_EXT "VK_KHR_display"
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  #define LAVA_KKHR_EXT "VK_KHR_wayland_surface"
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  #define LAVA_PLATFORM_LINUX
  #define LAVA_KHR_EXT "VK_KHR_xcb_surface"
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
  #define LAVA_PLATFORM_LINUX
  #define LAVA_KHR_EXT "VK_KHR_xlib_surface"
#elif defined(VK_USE_PLATFORM_IOS_MVK)
  #define LAVA_KHR_ExT "VK_MVK_ios_surface"
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
  #define LAVA_KHR_ExT "VK_MVK_macos_surface"
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#define LAVA_RUNTIME_ERROR(s) throw std::runtime_error( s );

#endif /* __LAVA_INCLUDES__ */