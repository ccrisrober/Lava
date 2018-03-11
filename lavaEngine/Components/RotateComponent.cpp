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

#include "RotateComponent.h"
#include <lavaEngine/Scenegraph/Node.h>
#include <lavaEngine/Mathematics/Mathf.h>

namespace lava
{
  namespace engine
  {
    RotateComponent::RotateComponent( const glm::vec3& axis, float speed )
      : _axis( axis )
      , _speed( speed )
      , _time( 0.0f )
    {
    }
    void RotateComponent::update( const lava::engine::Clock& clock )
    {
      node( )->rotate( _time * 2.0f - Mathf::PI, _axis, 
        Node::TransformSpace::Local );

      //.fromAxisAngle( _axis, _time * 2.0f * mb::Mathf::PI );
      _time += _speed * clock.getDeltaTime( );
    }
  }
}
