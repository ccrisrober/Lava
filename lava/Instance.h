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
  VkBool32 VKAPI_CALL debugMsgCallback( VkDebugReportFlagsEXT flags, 
    VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, 
    int32_t messageCode, const char* pLayerPrefix, const char* pMessage, 
    void* pUserData
  );

  class Instance : public std::enable_shared_from_this<Instance>,
    private NonCopyable<Instance>
  {
  public:
    Instance( const vk::Instance& i );
    LAVA_API
    static std::shared_ptr< Instance > createFromVkInstance( const vk::Instance& i );
    LAVA_API
    static std::shared_ptr< Instance > createDefault( const std::string& appName );
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