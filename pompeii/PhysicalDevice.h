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

#ifndef __POMPEII_PHYSICALDEVICE__
#define __POMPEII_PHYSICALDEVICE__

#include "includes.hpp"
#include "Device.h"

#include "noncopyable.hpp"

namespace pompeii
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
    POMPEII_API
    vk::ImageFormatProperties getImageFormatProperties( vk::Format format,
      vk::ImageType imageType, bool optimalTiling, vk::ImageUsageFlags usage,
      vk::ImageCreateFlags flags = { } ) const;
    // Returns a set of properties describing the physical device.
    POMPEII_API
    const vk::PhysicalDeviceProperties& getDeviceProperties( void ) const
    {
      return _deviceProperties;
    }

    // Returns a set of features that the application can use to check if a 
    //    specific feature is supported.
    POMPEII_API
    const vk::PhysicalDeviceFeatures& getDeviceFeatures( void ) const
    {
      return _deviceFeatures;
    }

    // Returns a set of properties describing the memory of the physical device.
    POMPEII_API
    const vk::PhysicalDeviceMemoryProperties& getMemoryProperties( void ) const
    {
      return _memoryProperties;
    }

    POMPEII_API
    std::vector<vk::SurfaceFormatKHR> getSurfaceFormats(
      const std::shared_ptr<Surface>& surface ) const;
    POMPEII_API
    std::vector<vk::SurfaceFormatKHR> getSurfaceFormats(
      const vk::SurfaceKHR& surface ) const;

    POMPEII_API
    bool supportSurfaceKHR( size_t queueFamilyIdx,
      const std::shared_ptr<Surface>& surface );

    POMPEII_API
    std::shared_ptr<Device> createDevice(
      const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
      const std::vector<std::string>& enabledLayerNames,
      const std::vector<std::string>& enabledExtensionNames,
      const vk::PhysicalDeviceFeatures& enabledFeatures = {} );

    POMPEII_API
    vk::SurfaceCapabilitiesKHR getSurfaceCapabilities(
      const std::shared_ptr<Surface>& surface ) const;

    POMPEII_API
    const std::vector< vk::PresentModeKHR >
      getSurfacePresentModes( const std::shared_ptr<Surface>& surface ) const;

    POMPEII_API
    vk::FormatProperties getFormatProperties( vk::Format format ) const
    {
      return _physicalDevice.getFormatProperties( format );
    }

    POMPEII_API
    bool extensionSupported( const std::string& extension )
    {
      return ( std::find( supportedExtensions.begin( ), 
        supportedExtensions.end( ), extension ) != supportedExtensions.end( ) );
    }

    POMPEII_API
    std::vector<vk::QueueFamilyProperties> getQueueFamilyProperties( void ) const
    {
      return _physicalDevice.getQueueFamilyProperties( );
    }

    POMPEII_API
    std::vector<uint32_t> getGraphicsPresentQueueFamilyIndices(
      const std::shared_ptr<Surface>& surface );

    POMPEII_API
    std::vector<uint32_t> getComputeQueueFamilyIndices( void );

	POMPEII_API
	std::shared_ptr<Instance> getInstance(void) const
	{
		return _instance;
	}

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

#endif /* __POMPEII_PHYSICALDEVICE__ */