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
    std::vector<vk::LayerProperties> properties =
      _physicalDevice.enumerateDeviceLayerProperties( );
    for ( const auto& prop : properties )
    {
      std::cout << "\t" << prop.layerName << std::endl;
    }

    _deviceProperties = _physicalDevice.getProperties( );
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

    int deviceType = (int)_deviceProperties.deviceType;
    printf( "Driver Version: %d\n", _deviceProperties.driverVersion );
    printf( "Device Name:    %s\n", _deviceProperties.deviceName );
    printf( "Device Type:    %s(%d)\n", devTypeStr, deviceType );
    printf( "API Version:    %d.%d.%d\n",
      // See note below regarding this:
      ( _deviceProperties.apiVersion >> 22 ) & 0x3FF,
      ( _deviceProperties.apiVersion >> 12 ) & 0x3FF,
      ( _deviceProperties.apiVersion & 0xFFF ) );

    _deviceFeatures = _physicalDevice.getFeatures( );
    _memoryProperties = _physicalDevice.getMemoryProperties( );


    _deviceFeatures.geometryShader = VK_TRUE;
    _deviceFeatures.tessellationShader = VK_TRUE;
    _deviceFeatures.depthClamp = VK_TRUE;
    _deviceFeatures.fillModeNonSolid = VK_TRUE;
    _deviceFeatures.multiViewport = VK_TRUE;
    _deviceFeatures.shaderClipDistance = VK_TRUE;


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

  }

  std::vector<vk::SurfaceFormatKHR> PhysicalDevice::getSurfaceFormats( 
    const std::shared_ptr<Surface>& surface ) const
  {
    return _physicalDevice.getSurfaceFormatsKHR(
      static_cast< vk::SurfaceKHR >( *surface ) );
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
    return _physicalDevice.getSurfaceCapabilitiesKHR(
      static_cast< vk::SurfaceKHR >( *surface ) );
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