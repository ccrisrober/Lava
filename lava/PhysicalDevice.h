#ifndef __LAVA_PHYSICALDEVICE__
#define __LAVA_PHYSICALDEVICE__

#include "includes.hpp"
#include "Surface.h"
#include "Device.h"

#include "noncopyable.hpp"

namespace lava
{
  class Instance;
  class PhysicalDevice : public std::enable_shared_from_this<PhysicalDevice>,
    private NonCopyable<PhysicalDevice>
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
      const std::shared_ptr<Surface>& surface ) const
    {
      return _physicalDevice.getSurfaceFormatsKHR(
        static_cast< vk::SurfaceKHR >( *surface ) );
    }

    LAVA_API
    DeviceRef createDevice(
      const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
      const std::vector<std::string>& enabledLayerNames,
      const std::vector<std::string>& enabledExtensionNames,
      const vk::PhysicalDeviceFeatures& enabledFeatures = {} );

    bool getSurfaceSupport( uint32_t qfi,
      const std::shared_ptr<Surface>& surface ) const
    {
      return !!_physicalDevice.getSurfaceSupportKHR( qfi,
        static_cast< vk::SurfaceKHR >( *surface ) );
    }

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
      const std::shared_ptr<Surface>& surface )
    {
      std::vector<vk::QueueFamilyProperties> props =
        _physicalDevice.getQueueFamilyProperties( );
      assert( !props.empty( ) );

      std::vector<uint32_t> indices;
      for ( size_t i = 0; i < props.size( ); ++i )
      {
        if ( ( props[ i ].queueFlags & vk::QueueFlagBits::eGraphics ) &&
          this->getSurfaceSupport( i, surface ) )
        {
          indices.push_back( i );
        }
      }
      return indices;
    }

    std::vector<uint32_t> getComputeQueueFamilyIndices( void  )
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