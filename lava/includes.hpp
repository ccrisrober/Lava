/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

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

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

#if defined( __MINGW32__ ) || defined( WIN32 ) || defined( __WIN32__ )
  #define LAVA_PLATFORM_WINDOWS
  #define LAVA_KHR_EXT "VK_KHR_win32_surface"
/*#elif defined(VK_KHR_android_surface)
  #define LAVA_PLATFORM_ANDROID
  #define LAVA_KHR_EXT "VK_KHR_android_surface"
#elif defined(VK_KHR_display)
  #define LAVA_KKHR_EXT "VK_KHR_display"
#elif defined(VK_KHR_wayland_surface)
  #define LAVA_KHR_EXT "VK_KHR_wayland_surface"*/
#elif defined(_GLFW_X11)
  #define LAVA_PLATFORM_LINUX
  #define LAVA_KHR_EXT "VK_KHR_xcb_surface"
#else //elif defined(VK_KHR_xlib_surface)
  #define LAVA_PLATFORM_LINUX
  #define LAVA_KHR_EXT "VK_KHR_xcb_surface"
/*#elif defined(VK_MVK_ios_surface)
  #define LAVA_KHR_ExT "VK_MVK_ios_surface"
#elif defined(VK_MVK_macos_surface)
  #define LAVA_KHR_ExT "VK_MVK_macos_surface"*/
#endif

#include <vulkan/vulkan.hpp>

#include <iostream>

#define LAVA_RUNTIME_ERROR(s) throw std::runtime_error( s );

#endif /* __LAVA_INCLUDES__ */
