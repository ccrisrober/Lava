#include "PhysicalDevice.h"

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

    VkPhysicalDeviceProperties deviceProperties = _physicalDevice.getProperties( );
    printf( "Driver Version: %d\n", deviceProperties.driverVersion );
    printf( "Device Name:    %s\n", deviceProperties.deviceName );
    printf( "Device Type:    %s(%d)\n", devTypeStr, deviceProperties.deviceType );
    printf( "API Version:    %d.%d.%d\n",
      // See note below regarding this:
      ( deviceProperties.apiVersion >> 22 ) & 0x3FF,
      ( deviceProperties.apiVersion >> 12 ) & 0x3FF,
      ( deviceProperties.apiVersion & 0xFFF ) );

    _deviceFeatures = _physicalDevice.getFeatures( );
    _memoryProperties = _physicalDevice.getMemoryProperties( );


    _deviceFeatures.geometryShader = VK_TRUE;
    _deviceFeatures.tessellationShader = VK_TRUE;
    _deviceFeatures.depthClamp = VK_TRUE;


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

  DeviceRef PhysicalDevice::createDevice(
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
}