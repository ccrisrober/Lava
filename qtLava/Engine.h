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