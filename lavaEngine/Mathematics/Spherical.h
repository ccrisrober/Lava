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

#ifndef __LAVAENGINE_SPHERICAL__
#define __LAVAENGINE_SPHERICAL__

#include <lavaEngine/api.h>

#include "../glm_config.h"

namespace lava
{
  namespace engine
  {
    class Spherical
    {
    public:
      LAVAENGINE_API
      glm::vec3 toCartesian( void );
      LAVAENGINE_API
      static Spherical cartesianToSpherical( glm::vec3 cartCoords );
      LAVAENGINE_API
      static glm::vec3 sphericalToCartesian( float r, float p, float e );
    protected:
      float _radius;
      float _polar;
      float _elevation;
    };
  }
}

#endif /* __LAVAENGINE_SPHERICAL__ */
