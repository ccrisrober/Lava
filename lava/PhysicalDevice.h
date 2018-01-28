/**
 * Copyright (c) 2017, Lava
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

#ifndef __LAVA_PHYSICALDEVICE__
#define __LAVA_PHYSICALDEVICE__

#include "includes.hpp"
#include "Device.h"

#include "noncopyable.hpp"

namespace lava
{
  class Instance;
  class Surface;
  class PhysicalDevice
    : public std::enable_shared_from_this<PhysicalDevice>
    , private NonCopyable<PhysicalDevice>
  {
  public:
    PhysicalDevice( const std::shared_ptr< Instance >& instance,
      const vk::PhysicalDevice physicalDevice );
    virtual ~PhysicalDevice( void );

    inline operator vk::PhysicalDevice( void )
    {
      return _physicalDevice;
    }
    // Returns a set of properties describing the physical device.
    const vk::PhysicalDeviceProperties& getDeviceProperties( void ) const
    {
      return _deviceProperties;
    }

    // Returns a set of features that the application can use to check if a 
    //    specific feature is supported.
    const vk::PhysicalDeviceFeatures& getDeviceFeatures( void ) const
    {
      return _deviceFeatures;
    }

    // Returns a set of properties describing the memory of the physical device.
    const vk::PhysicalDeviceMemoryProperties& getMemoryProperties( void ) const
    {
      return _memoryProperties;
    }

    std::vector<vk::SurfaceFormatKHR> getSurfaceFormats(
      const std::shared_ptr<Surface>& surface ) const;

    bool supportSurfaceKHR( size_t queueFamilyIdx,
      const std::shared_ptr<Surface>& surface );

    LAVA_API
    std::shared_ptr<Device> createDevice(
      const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
      const std::vector<std::string>& enabledLayerNames,
      const std::vector<std::string>& enabledExtensionNames,
      const vk::PhysicalDeviceFeatures& enabledFeatures = {} );

    vk::SurfaceCapabilitiesKHR getSurfaceCapabilities(
      const std::shared_ptr<Surface>& surface ) const;

    vk::FormatProperties getFormatProperties( vk::Format format ) const
    {
      return _physicalDevice.getFormatProperties( format );
    }
    
    bool extensionSupported( const std::string& extension )
    {
      return ( std::find( supportedExtensions.begin( ), 
        supportedExtensions.end( ), extension ) != supportedExtensions.end( ) );
    }
    
    std::vector<vk::QueueFamilyProperties> getQueueFamilyProperties( void ) const
    {
      return _physicalDevice.getQueueFamilyProperties( );
    }

    std::vector<uint32_t> getGraphicsPresentQueueFamilyIndices(
      const std::shared_ptr<Surface>& surface );

    std::vector<uint32_t> getComputeQueueFamilyIndices( void );

  private:
    std::shared_ptr<Instance> _instance;
    vk::PhysicalDevice _physicalDevice;

    vk::PhysicalDeviceProperties _deviceProperties;
    vk::PhysicalDeviceFeatures _deviceFeatures;
    vk::PhysicalDeviceMemoryProperties _memoryProperties;
    
    //List of extensions supported by the device
    std::vector<std::string> supportedExtensions;
  };
}

#endif /* __LAVA_PHYSICALDEVICE__ */