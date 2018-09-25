/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#ifndef __POMPEII_INCLUDES__
#define __POMPEII_INCLUDES__

/*#if defined ( __ANDROID__ )
  #define POMPEII_PLATFORM_ANDROID
#elif defined( __APPLE__ )
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #define POMPEII_PLATFORM_IOS
  #else
    #define POMPEII_PLATFORM_OSX
  #endif
#elif defined( __MINGW32__ ) || defined( WIN32 ) || defined( __WIN32__ )
  #define POMPEII_PLATFORM_WINDOWS
#else
  #define POMPEII_PLATFORM_LINUX
#endif

#if defined( POMPEII_PLATFORM_WINDOWS ) || defined( POMPEII_PLATFORM_LINUX ) ||defined( POMPEII_PLATFORM_OSX )
  #define POMPEII_PLATFORM_DESKTOP
#endif

#if defined( POMPEII_PLATFORM_IOS ) || defined( POMPEII_PLATFORM_ANDROID )
  #define POMPEII_PLATFORM_MOBILE
#endif*/

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

#if defined( __MINGW32__ ) || defined( WIN32 ) || defined( __WIN32__ )
  #define POMPEII_PLATFORM_WINDOWS
  #define POMPEII_KHR_EXT "VK_KHR_win32_surface"
/*#elif defined(VK_KHR_android_surface)
  #define POMPEII_PLATFORM_ANDROID
  #define POMPEII_KHR_EXT "VK_KHR_android_surface"
#elif defined(VK_KHR_display)
  #define POMPEII_KKHR_EXT "VK_KHR_display"
#elif defined(VK_KHR_wayland_surface)
  #define POMPEII_KHR_EXT "VK_KHR_wayland_surface"*/
#elif defined(_GLFW_X11)
  #define POMPEII_PLATFORM_LINUX
  #define POMPEII_KHR_EXT "VK_KHR_xcb_surface"
#else //elif defined(VK_KHR_xlib_surface)
  #define POMPEII_PLATFORM_LINUX
  #define POMPEII_KHR_EXT "VK_KHR_xcb_surface"
/*#elif defined(VK_MVK_ios_surface)
  #define POMPEII_KHR_ExT "VK_MVK_ios_surface"
#elif defined(VK_MVK_macos_surface)
  #define POMPEII_KHR_ExT "VK_MVK_macos_surface"*/
#endif

#include <vulkan/vulkan.hpp>

#include <iostream>

#define POMPEII_RUNTIME_ERROR(s) throw std::runtime_error( s );

#endif /* __POMPEII_INCLUDES__ */
