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

#ifndef __POMPEIINENGINE_FETCH_LIGHTS__
#define __POMPEIINENGINE_FETCH_LIGHTS__

#include "Visitor.h"
#include <vector>
#include <functional>

#include <pompeiiEngine/api.h>

namespace pompeii
{
  namespace engine
  {
    class FetchLights :
      public Visitor
    {
    public:
      POMPEIIENGINE_API
      virtual void reset( void ) override;
      POMPEIIENGINE_API
      virtual void visitLight( Light* c ) override;
      POMPEIIENGINE_API
      bool hasLights( void ) const
      {
        return !_lights.empty( );
      }
      POMPEIIENGINE_API
      std::vector<Light*> lights( void ) const
      {
        return _lights;
      }
      POMPEIIENGINE_API
      void forEachLight( std::function<void( Light* )> cb );
    protected:
      std::vector<Light*> _lights;
    };
  }
}

#endif /* __POMPEIINENGINE_FETCH_LIGHTS__ */
