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

#ifndef __LAVANENGINE_FETCH_LIGHTS__
#define __LAVANENGINE_FETCH_LIGHTS__

#include "Visitor.h"
#include <vector>
#include <functional>

#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class FetchLights :
      public Visitor
    {
    public:
      LAVAENGINE_API
      virtual void reset( void ) override;
      LAVAENGINE_API
      virtual void visitLight( Light* c ) override;
      LAVAENGINE_API
      bool hasLights( void ) const
      {
        return !_lights.empty( );
      }
      LAVAENGINE_API
      std::vector<Light*> lights( void ) const
      {
        return _lights;
      }
      LAVAENGINE_API
      void forEachLight( std::function<void( Light* )> cb );
    protected:
      std::vector<Light*> _lights;
    };
  }
}

#endif /* __LAVANENGINE_FETCH_LIGHTS__ */
