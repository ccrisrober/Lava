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

#include "Engine.h"

#include <iostream>

#include <QLoggingCategory>

namespace lava
{
  Engine::Engine( const CreateInfo &info )
    : info( info )
  {
    createInstance( );
  }

  Engine::~Engine( void )
  {

    //instance.destroy( nullptr );
  }

  void Engine::createInstance( void )
  {
    vk::ApplicationInfo appInfo(
      info.appInfo.c_str( ),
      VK_MAKE_VERSION( 1, 0, 0 ),
      "LavaEngine",
      VK_MAKE_VERSION( 1, 0, 0 ),
      VK_API_VERSION_1_0
    );

#ifndef NDEBUG
    QLoggingCategory::setFilterRules( QStringLiteral( "qt.vulkan=true" ) );
#endif

    vk::InstanceCreateInfo ici;
    ici.setPApplicationInfo( &appInfo );

    // Extensions

    auto extensionsdExts = vk::enumerateInstanceExtensionProperties( );
    std::cout << "Available Extensions:" << std::endl;
    for ( const auto &extension : extensionsdExts )
    {
      std::cout << "\t" << extension.extensionName << std::endl;
    }

    std::vector<const char *> requiredExts;
    {
      for ( const auto &extension : info.requiredInstanceExtensions )
      {
        requiredExts.push_back( extension.c_str( ) );
      }

      if ( info.enableValidationLayers )
      {
        requiredExts.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
      }
    }

    ici.setEnabledExtensionCount( static_cast< uint32_t >( requiredExts.size( ) ) );
    ici.setPpEnabledExtensionNames( requiredExts.data( ) );

    std::vector<const char* > layers;
    {
      layers = {
  #ifndef NDEBUG
    #ifndef Q_OS_ANDROID
        "VK_LAYER_LUNARG_standard_validation"
    #else
        "VK_LAYER_GOOGLE_threading",
          "VK_LAYER_LUNARG_parameter_validation",
          "VK_LAYER_LUNARG_object_tracker",
          "VK_LAYER_LUNARG_core_validation",
          "VK_LAYER_LUNARG_swapchain",
          "VK_LAYER_GOOGLE_unique_objects"
    #endif
  #endif
      };
      if ( info.enableRenderdoc )
      {
        // TODO
      }
    }
    ici.setEnabledLayerCount( static_cast< uint32_t >( layers.size( ) ) );
    ici.setPpEnabledLayerNames( layers.data( ) );

    instance = vk::createInstance( ici );
  }
}