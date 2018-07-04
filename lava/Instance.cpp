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

#include "Instance.h"

#include <iostream>

#include <lava/version.h>

PFN_vkCreateDebugReportCallbackEXT  pfnVkCreateDebugReportCallbackEXT;
PFN_vkDestroyDebugReportCallbackEXT pfnVkDestroyDebugReportCallbackEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
  VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, 
  const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	return pfnVkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, 
  VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	pfnVkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

#include <sstream>

namespace lava
{
  VKAPI_ATTR VkBool32 VKAPI_CALL debugMsgCallback( VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT, uint64_t,
    size_t, int32_t msgCode, const char* pLayerPrefix,
    const char* pMsg, void* )
  {
    //std::cerr << "vulkan: " << pMsg << std::endl;
    //return VK_FALSE;

    std::stringstream message;

    // Determine prefix
    if ( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
    {
      message << "ERROR";
    }

    if ( flags & VK_DEBUG_REPORT_WARNING_BIT_EXT )
    {
      message << "WARNING";
    }

    if ( flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT )
    {
      message << "PERFORMANCE";
      
      message << ": [" << pLayerPrefix << "] Code " << msgCode << ": "
        << pMsg << std::endl;
      std::cerr << message.str( ) << std::endl;
      return VK_TRUE;
    }

    if ( flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT )
    {
      message << "INFO";
    }

    if ( flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT )
    {
      message << "DEBUG";
    }

    message << ": [" << pLayerPrefix << "] Code " << msgCode << ": "
      << pMsg << std::endl;

    if ( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
    {
      std::cerr << message.str( ) << std::endl;
    }
    else if ( flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags &
      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT )
    {
      std::cerr << message.str( ) << std::endl;
    }
    else
    {
      std::cerr << message.str( ) << std::endl;
    }

    std::cerr << message.str( ) << std::endl;
    assert( !message );

    // Abort calls that caused a validation message
    return VK_FALSE;
  }

  Instance::Instance( const vk::Instance& i )
  {
    _instance = i;
    _physicalDevices = _instance.enumeratePhysicalDevices( );
    _physicalDevicesCache.resize( _physicalDevices.size( ) );

    static bool initialized = false;
    if ( !initialized )
    {
      pfnVkCreateDebugReportCallbackEXT = reinterpret_cast
        < PFN_vkCreateDebugReportCallbackEXT >(
          _instance.getProcAddr( "vkCreateDebugReportCallbackEXT" ) );
      pfnVkDestroyDebugReportCallbackEXT = reinterpret_cast
        < PFN_vkDestroyDebugReportCallbackEXT >(
          _instance.getProcAddr( "vkDestroyDebugReportCallbackEXT" ) );
      initialized = true;
    }
  }

  std::shared_ptr< Instance > Instance::createFromVkInstance( const vk::Instance& i )
  {
    return std::make_shared< Instance >( i );
  }

  std::shared_ptr<Instance> Instance::createDefault( const std::string& appName )
  {
    vk::ApplicationInfo appInfo(
      appName.c_str( ),
      VK_MAKE_VERSION(
        LAVA_VERSION_MAJOR,
        LAVA_VERSION_MINOR,
        LAVA_VERSION_PATCH
      ),
      "LavaEngine",
      VK_MAKE_VERSION(
        LAVA_VERSION_MAJOR,
        LAVA_VERSION_MINOR,
        LAVA_VERSION_PATCH
      ),
      VK_API_VERSION_1_0
    );

    std::vector<const char*> layers =
    {
#ifndef NDEBUG
#ifndef __ANDROID__
      "VK_LAYER_LUNARG_standard_validation"
#else
      "VK_LAYER_GOOGLE_threading",
      "VK_LAYER_LUNARG_parameter_validation",
      "VK_LAYER_LUNARG_object_tracker",
      "VK_LAYER_LUNARG_core_validation",
      "VK_LAYER_LUNARG_swapchain",
      "VK_LAYER_GOOGLE_unique_objects"
#endif
#endif
    };
    std::vector<const char*> extensions =
    {
      VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
      LAVA_KHR_EXT, // OS specific surface extension
      VK_EXT_DEBUG_REPORT_EXTENSION_NAME
    };

    vk::InstanceCreateInfo ci(
      vk::InstanceCreateFlags( ),
      &appInfo,
      layers.size( ),
      layers.data( ),
      extensions.size( ),
      extensions.data( )
    );
    return std::make_shared<Instance>( ci );
  }

  std::shared_ptr<Instance> Instance::create( const std::string& appName )
  {
    vk::ApplicationInfo appInfo(
      appName.c_str( ),
      VK_MAKE_VERSION( 
        LAVA_VERSION_MAJOR, 
        LAVA_VERSION_MINOR,
        LAVA_VERSION_PATCH
      ),
      "LavaEngine",
      VK_MAKE_VERSION( 
        LAVA_VERSION_MAJOR, 
        LAVA_VERSION_MINOR,
        LAVA_VERSION_PATCH
      ),
      VK_API_VERSION_1_0
    );
    vk::InstanceCreateInfo ci(
      vk::InstanceCreateFlags( ),
      &appInfo,
      0,
      nullptr,
      0,
      nullptr
    );
    return std::make_shared<Instance>( ci );
  }
  
  std::shared_ptr<Instance> Instance::create( const vk::InstanceCreateInfo& ci )
  {
    return std::make_shared<Instance>( ci );
  }

  Instance::Instance( const vk::InstanceCreateInfo& ci )
  {
    _instance = vk::createInstance( ci );
    _physicalDevices = _instance.enumeratePhysicalDevices( );
    _physicalDevicesCache.resize( _physicalDevices.size( ) );

    static bool initialized = false;
    if ( !initialized )
    {
      pfnVkCreateDebugReportCallbackEXT = reinterpret_cast
        < PFN_vkCreateDebugReportCallbackEXT >(
        _instance.getProcAddr( "vkCreateDebugReportCallbackEXT" ) );
      pfnVkDestroyDebugReportCallbackEXT = reinterpret_cast
        < PFN_vkDestroyDebugReportCallbackEXT >(
        _instance.getProcAddr( "vkDestroyDebugReportCallbackEXT" ) );
      initialized = true;
    }
  }
  Instance::~Instance( void )
  {
    destroy( );
  }
  void Instance::destroy( void )
  {
#ifndef NDEBUG
    if ( _debugCallback )
    {
      _instance.destroyDebugReportCallbackEXT( _debugCallback );
    }
#endif
    if ( _instance )
    {
      _physicalDevices.clear( );
      _instance.destroy( );
      //_instance = VK_NULL_HANDLE;
    }
  }
  void Instance::createDebugReportCallback(
    const vk::DebugReportCallbackCreateInfoEXT& debugInfo )
  {
#ifndef NDEBUG
    _debugCallback = _instance.createDebugReportCallbackEXT( debugInfo );
#endif
  }


  uint32_t Instance::getPhysicalDeviceCount( ) const
  {
    return( _physicalDevices.size( ) );
  }

  std::shared_ptr<PhysicalDevice> Instance::getPhysicalDevice( uint32_t idx )
  {
    assert( idx < _physicalDevices.size( ) );

    auto phyDev = _physicalDevicesCache[ idx ].lock( );
    if ( !phyDev )
    {
      phyDev = std::make_shared<PhysicalDevice>( shared_from_this( ), 
        _physicalDevices[ idx ] );
      _physicalDevicesCache[ idx ] = phyDev;
    }
    return phyDev;
  }
}
