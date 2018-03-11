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

#ifndef __LAVA_ENGINE_FETCHCAMERAS__
#define __LAVA_ENGINE_FETCHCAMERAS__

#include <lavaEngine/Scenegraph/Camera.h>
#include <lavaEngine/api.h>
#include <vector>
#include <functional>

namespace lava
{
  namespace engine
  {
    class FetchCameras:
      public Visitor
    {
    public:
      LAVAENGINE_API
      virtual void reset( void ) override;
      LAVAENGINE_API
      virtual void visitCamera( Camera* c ) override;
      LAVAENGINE_API
      bool hasCameras( void ) const
      {
        return !_cameras.empty( );
      }
      LAVAENGINE_API
      void forEachCameras( std::function<void( Camera* )> cb );
    protected:
      std::vector< Camera* > _cameras;
      };
  }
}

#endif /* __LAVA_ENGINE_FETCHCAMERAS__ */