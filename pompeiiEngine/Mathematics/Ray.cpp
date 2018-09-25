/**
 * Copyright (c) 2017, Monkey Brush
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

#include "Ray.h"

namespace pompeii
{
  namespace engine
  {
    Ray::Ray( const glm::vec3& origin, const glm::vec3& direction )
      : _origin( origin )
      , _direction( direction )
    {
    }
    glm::vec3 Ray::origin( void ) const
    {
      return _origin;
    }
    glm::vec3 Ray::direction( void ) const
    {
      return _direction;
    }
    void Ray::origin( const glm::vec3& ori )
    {
      _origin = ori;
    }
    void Ray::direction( const glm::vec3& dir )
    {
      _direction = dir;
    }
    glm::vec3 Ray::getPoint( const float& t )
    {
      return glm::vec3(
        _origin.x + t * _direction.x,
        _origin.y + t * _direction.y,
        _origin.z + t * _direction.z
      );
    }
    /*bool Ray::intersect( const BoundingVolume* )
    {
      // TODO
      return true;
    }*/
  }
}
