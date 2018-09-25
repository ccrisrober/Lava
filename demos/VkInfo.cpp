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

#include <pompeii/pompeii.h>
#include <vulkan/vulkan.hpp>

int main( void )
{
  try
  {
    auto layerProps = vk::enumerateInstanceLayerProperties( );
    std::cout << "Layers: " << layerProps.size( ) << std::endl;
    uint32_t i = 0;
    for ( const auto& prop : layerProps )
    {
      std::cout << "Layer " << i << ":" << std::endl;
      std::cout << "\tlayerName             : " 
        << prop.layerName << std::endl;
      std::cout << "\tspecVersion           : " 
        << prop.specVersion << std::endl;
      std::cout << "\timplementationVersion : " 
        << prop.implementationVersion << std::endl;
      std::cout << "\tdescription           : " 
        << prop.description << std::endl;
      ++i;
    }

    auto instanceExts = vk::enumerateInstanceExtensionProperties( );
    std::cout << "InstanceExtensions : " << instanceExts.size( ) << std::endl;
    for ( const auto& ext : instanceExts )
    {
      std::cout << "\t" << ext.extensionName << " (Version " 
        << ext.specVersion << ")" << std::endl;
    }

    std::shared_ptr<pompeii::Instance> instance = pompeii::Instance::create( "" );

    uint32_t phy_dev_count = instance->getPhysicalDeviceCount( );
    std::cout << "PhysicalDeviceCount : " << phy_dev_count << std::endl;
    for ( i = 0; i < phy_dev_count; ++i )
    {
      auto pd = instance->getPhysicalDevice( i );
      vk::PhysicalDeviceProperties props = pd->getDeviceProperties( );

      std::cout << "Device " << i << ":" << std::endl;
      std::cout << "\tAPI Version    : " << props.apiVersion << std::endl;
      std::cout << "\tDriver Version : " << props.driverVersion << std::endl;
      std::cout << "\tVendor ID      : " << props.vendorID << std::endl;
      std::cout << "\tDevice ID      : " << props.deviceID << std::endl;
      std::cout << "\tDevice Type    : " << vk::to_string( props.deviceType ) 
        << std::endl;
      std::cout << "\tDevice Name    : " << props.deviceName << std::endl;

      std::cout << "\tlimits: " << std::endl;
      std::cout << "\t\tMax Image Dimension 1D: "
        << props.limits.maxImageDimension1D << std::endl;
      std::cout << "\t\tMax Image Dimension 2D: "
        << props.limits.maxImageDimension2D << std::endl;
      std::cout << "\t\tMax Image Dimension 3D: "
        << props.limits.maxImageDimension3D << std::endl;
      std::cout << "\t\tMax Push Constant Size: "
        << props.limits.maxPushConstantsSize << std::endl;
      std::cout << "\t\tMax Uniform Buffer Range: "
        << props.limits.maxUniformBufferRange << std::endl;
      std::cout << "\t\tMax Storage Buffer Range: "
        << props.limits.maxStorageBufferRange << std::endl;
      std::cout << "\t\tMax Sampler Anisotropy: "
        << props.limits.maxSamplerAnisotropy << std::endl;
      std::cout << "\t\tMax Compute Shared Memory Size: "
        << props.limits.maxComputeSharedMemorySize << std::endl;
      std::cout << "\t\tMax Compute Work Group Count: "
        << props.limits.maxComputeWorkGroupCount[ 0 ] << ", "
        << props.limits.maxComputeWorkGroupCount[ 1 ] << ", "
        << props.limits.maxComputeWorkGroupCount[ 2 ] << std::endl;
      std::cout << "\t\tMax Compute Work Group Invocations: "
        << props.limits.maxComputeWorkGroupInvocations << std::endl;
      std::cout << "\t\tMax Compute Work Group Size: "
        << props.limits.maxComputeWorkGroupSize[ 0 ] << ", "
        << props.limits.maxComputeWorkGroupSize[ 1 ] << ", "
        << props.limits.maxComputeWorkGroupSize[ 2 ] << std::endl;

      auto qProps = static_cast< vk::PhysicalDevice >( *pd )
        .getQueueFamilyProperties( );
      std::cout << "\tQueue Family Property Count : " 
        << qProps.size( ) << std::endl;
      uint32_t q = 0;
      for ( const auto& prop : qProps )
      {
        std::cout << "\t\tFamily " << q << ":" << std::endl;
        std::cout << "\t\t\tQueue Flags                    : " 
          << vk::to_string( prop.queueFlags ) << std::endl;
        std::cout << "\t\t\tQueue Count                    : " 
          << prop.queueCount << std::endl;
        std::cout << "\t\t\tTimestampe Valid Bits          : " 
          << prop.timestampValidBits << std::endl;
        std::cout << "\t\t\tMin Image Transfer Granularity : " 
          << prop.minImageTransferGranularity.width << ", " <<
          prop.minImageTransferGranularity.height << ", " 
            << prop.minImageTransferGranularity.depth << std::endl;
        ++q;
      }


      auto devExts = static_cast< vk::PhysicalDevice >( *pd )
        .enumerateDeviceExtensionProperties( );

      std::cout << "\tDeviceExtensions : " << devExts.size( ) << std::endl;
      for ( const auto& ext : devExts )
      {
        std::cout << "\t\t" << ext.extensionName << " (Version " 
          << ext.specVersion << ")" << std::endl;
      }
    }
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  system( "PAUSE" );
  return 0;
}