#ifndef __LAVA_INSTANCE__
#define __LAVA_INSTANCE__

#include "includes.hpp"
#include "PhysicalDevice.h"
#include "Surface.h"
#include <memory>

#include "noncopyable.hpp"

#include <lava/api.h>

namespace lava
{
  VkBool32 VKAPI_CALL debugMsgCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
    uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
    const char* pMessage, void* pUserData );
  class Instance : public std::enable_shared_from_this<Instance>,
    private NonCopyable<Instance>
  {
  public:
    LAVA_API
      static std::shared_ptr<Instance> create( const std::string& appName );
    LAVA_API
      static std::shared_ptr<Instance> create( const vk::InstanceCreateInfo& ci );
    LAVA_API
      Instance( const vk::InstanceCreateInfo& ci );
    LAVA_API
      virtual ~Instance( void );

    LAVA_API
      inline operator vk::Instance( void )
    {
      return _instance;
    }

    void destroy( void );

    LAVA_API
      void createDebugReportCallback(
        const vk::DebugReportCallbackCreateInfoEXT& debugInfo );

    LAVA_API
      std::shared_ptr<Surface> createSurfaceKHR( GLFWwindow* window )
    {
      // Surface KHR
      VkSurfaceKHR surface;
      if ( glfwCreateWindowSurface( VkInstance( _instance ),
        window, nullptr, &surface ) != VK_SUCCESS )
      {
        throw std::runtime_error( "failed to create window surface!" );
      }
      return std::make_shared<Surface>( shared_from_this( ), vk::SurfaceKHR( surface ) );
    }
    LAVA_API
      uint32_t getPhysicalDeviceCount( void ) const;
    LAVA_API
      std::shared_ptr<PhysicalDevice> getPhysicalDevice( uint32_t index );
  private:
    std::vector<vk::PhysicalDevice> _physicalDevices;
    std::vector<std::weak_ptr<PhysicalDevice>> _physicalDevicesCache;
    vk::Instance _instance;
#ifndef NDEBUG
    vk::DebugReportCallbackEXT _debugCallback = nullptr;
#endif
  };
  typedef std::shared_ptr< Instance > InstanceRef;
}

#endif /* __LAVA_INSTANCE__ */