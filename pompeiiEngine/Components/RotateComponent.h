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

#ifndef __POMPEIIENGINE_ROTATE_COMPONENT__
#define __POMPEIIENGINE_ROTATE_COMPONENT__

#include "Component.h"
#include <pompeiiEngine/api.h>

#include "../glm_config.h"

namespace pompeii
{
  namespace engine
  {
    class RotateComponent : public Component
    {
      IMPLEMENT_COMPONENT( RotateComponent )
    public:
      POMPEIIENGINE_API
  	  RotateComponent( const glm::vec3& axis, float speed );
      POMPEIIENGINE_API
    	virtual void update( const pompeii::engine::Clock& clock ) override;
  	protected:
  	  glm::vec3 _axis;
  	  float _speed;
  	  float _time;
    };
  }
}

#endif /* __POMPEIIENGINE_ROTATE_COMPONENT__ */
