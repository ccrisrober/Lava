#pragma once

#include <lava/lava.h>
#include <glfwLava/api.h>

namespace lava
{
  class Engine
  {
  public:
    struct CreateInfo
    {
      std::string appInfo;
      bool enableValidationLayers = false;
      bool enableRenderdoc = false;
      std::set<std::string> requiredInstanceExtensions;
      std::set<std::string> requiredDeviceExtensions;

      CreateInfo( void ) = default;
    };
    GLFWLAVA_API
    Engine( const CreateInfo &info );
    GLFWLAVA_API
    ~Engine( void );
    GLFWLAVA_API
    const vk::Instance& GetVkInstance( void )	const { return instance; }
  private:
    void createInstance( void );

    const CreateInfo info;

    vk::Instance instance;
  };
}