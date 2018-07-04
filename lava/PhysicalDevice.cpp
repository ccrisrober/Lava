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

#include "PhysicalDevice.h"

#include "Surface.h"

#include "Instance.h"

#include <iostream>

namespace lava
{
  PhysicalDevice::PhysicalDevice( const std::shared_ptr< Instance >& instance,
    const vk::PhysicalDevice physicalDevice )
    : _instance( instance )
    , _physicalDevice( physicalDevice )
  {
    _deviceProperties = _physicalDevice.getProperties( );

#ifndef NDEBUG
    std::vector<vk::LayerProperties> properties =
      _physicalDevice.enumerateDeviceLayerProperties( );
    for ( const auto& prop : properties )
    {
      std::cout << "\t" << prop.layerName << std::endl;
    }

    const char* devTypeStr = "";
    switch ( _deviceProperties.deviceType )
    {
      case vk::PhysicalDeviceType::eOther:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_OTHER";
        break;
      case vk::PhysicalDeviceType::eIntegratedGpu:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
        break;
      case vk::PhysicalDeviceType::eDiscreteGpu:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
        break;
      case vk::PhysicalDeviceType::eVirtualGpu:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
        break;
      case vk::PhysicalDeviceType::eCpu:
        devTypeStr = "VK_PHYSICAL_DEVICE_TYPE_CPU";
        break;
    }

    auto vendorName = [ ]( uint32_t vendorId ) -> std::string
    {
      switch ( vendorId )
      {
      case 0x10DE:
        return "Nvidia";
      case 0x1002:
      case 0x1022:
        return "AMD";
      case 0x163C:
      case 0x8086:
      case 0x8087:
        return "Intel";
      default:
        return "UNKNOWN";
      }
    };

    auto version = [ ]( uint32_t version_ ) -> std::string {
      return std::to_string( VK_VERSION_MAJOR( version_ ) ) + "." + 
        std::to_string( VK_VERSION_MINOR( version_ ) ) + "." +
        std::to_string( VK_VERSION_PATCH( version_ ) );
    };

    std::cout << "Hardware/software information" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << "  Vendor:           " << vendorName( _deviceProperties.vendorID )
      << " (ID: " << _deviceProperties.vendorID << ")" << std::endl;
    std::cout << "  Device:           " << _deviceProperties.deviceName
      << " (ID: " << _deviceProperties.deviceID << ")" << std::endl;
    std::cout << "  Device type:      " << 
      devTypeStr/*vk::to_string( _deviceProperties.deviceType )*/ << std::endl;
    std::cout << "  API version:      " << 
      version( _deviceProperties.apiVersion ) << std::endl;
    std::cout << "  Driver version:   " << 
      version( _deviceProperties.driverVersion ) << std::endl;
    std::cout << std::endl;
#endif

    _deviceFeatures = _physicalDevice.getFeatures( );
    _memoryProperties = _physicalDevice.getMemoryProperties( );


    /*_deviceFeatures.geometryShader = VK_TRUE;
    _deviceFeatures.tessellationShader = VK_TRUE;
    _deviceFeatures.depthClamp = VK_TRUE;
    _deviceFeatures.fillModeNonSolid = VK_TRUE;
    _deviceFeatures.multiViewport = VK_TRUE;
    _deviceFeatures.shaderClipDistance = VK_TRUE;*/


    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      _physicalDevice.getQueueFamilyProperties( );

    auto exts = _physicalDevice.enumerateDeviceExtensionProperties( );
    for ( const auto& ext : exts )
    {
      supportedExtensions.push_back( ext.extensionName );
    }
  }
  PhysicalDevice::~PhysicalDevice( void )
  {
    // Nothing to do here
  }

  vk::ImageFormatProperties PhysicalDevice::getImageFormatProperties( 
    vk::Format format, vk::ImageType imageType, bool optimalTiling, 
    vk::ImageUsageFlags usage, vk::ImageCreateFlags flags ) const
  {
    return _physicalDevice.getImageFormatProperties( format, imageType,
      optimalTiling ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear, usage, flags );
  }

  std::vector<vk::SurfaceFormatKHR> PhysicalDevice::getSurfaceFormats( 
    const std::shared_ptr<Surface>& surface ) const
  {
    return getSurfaceFormats( *surface );
  }

  std::vector<vk::SurfaceFormatKHR> PhysicalDevice::getSurfaceFormats(
    const vk::SurfaceKHR& surface ) const
  {
    return _physicalDevice.getSurfaceFormatsKHR( surface );
  }

  bool PhysicalDevice::supportSurfaceKHR( size_t queueFamilyIdx,
    const std::shared_ptr<Surface>& surf )
  {
    return _physicalDevice.getSurfaceSupportKHR( queueFamilyIdx, *surf );
  }

  std::shared_ptr<Device> PhysicalDevice::createDevice(
    const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
    const std::vector<std::string>& enabledLayerNames,
    const std::vector<std::string>& enabledExtensionNames,
    const vk::PhysicalDeviceFeatures& enabledFeatures )
  {
#ifndef NDEBUG
    std::vector<std::string> enabledLayerNames_( enabledLayerNames );

    enabledLayerNames_.push_back( "VK_LAYER_LUNARG_standard_validation" );

    return Device::create( shared_from_this( ), queueCreateInfos,
      enabledLayerNames_, enabledExtensionNames, enabledFeatures );
#else
    return Device::create( shared_from_this( ), queueCreateInfos,
      enabledLayerNames, enabledExtensionNames, enabledFeatures );
#endif
  }
  vk::SurfaceCapabilitiesKHR PhysicalDevice::getSurfaceCapabilities(
    const std::shared_ptr<Surface>& surface ) const
  {
    return _physicalDevice.getSurfaceCapabilitiesKHR( *surface );
  }
  
  const std::vector< vk::PresentModeKHR >
    PhysicalDevice::getSurfacePresentModes( 
      const std::shared_ptr<Surface>& surface ) const
  {
    return _physicalDevice.getSurfacePresentModesKHR( *surface );
  }

  std::vector<uint32_t> PhysicalDevice::getGraphicsPresentQueueFamilyIndices( 
    const std::shared_ptr<Surface>& surface )
  {
    std::vector<vk::QueueFamilyProperties> props =
      _physicalDevice.getQueueFamilyProperties( );
    assert( !props.empty( ) );

    std::vector<uint32_t> indices;
    for ( size_t i = 0; i < props.size( ); ++i )
    {
      if ( ( props[ i ].queueFlags & vk::QueueFlagBits::eGraphics ) &&
        this->supportSurfaceKHR( i, surface ) )
      {
        indices.push_back( i );
      }
    }
    return indices;
  }
  std::vector<uint32_t> PhysicalDevice::getComputeQueueFamilyIndices( void )
  {
    std::vector<vk::QueueFamilyProperties> props =
      _physicalDevice.getQueueFamilyProperties( );
    assert( !props.empty( ) );

    std::vector<uint32_t> indices;
    for ( size_t i = 0; i < props.size( ); ++i )
    {
      if ( props[ i ].queueFlags & vk::QueueFlagBits::eCompute )
      {
        indices.push_back( i );
      }
    }
    return indices;
  }
}