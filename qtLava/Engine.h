#pragma once

#include <lava/lava.h>
#include <qtLava/api.h>

#include <set>

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
    QTLAVA_API
    Engine( const CreateInfo &info );
    QTLAVA_API
    ~Engine( void );
    QTLAVA_API
    const vk::Instance& GetVkInstance( void )	const { return instance; }
  private:
    void createInstance( void );

    const CreateInfo info;

    vk::Instance instance;
  };
}