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

#ifndef __POMPEII_ENGINE_LIGHT__
#define __POMPEII_ENGINE_LIGHT__

#include "Group.h"
#include <pompeiiEngine/api.h>

namespace pompeii
{
	namespace engine
	{
		typedef glm::vec3 Color;
		class Light: public Node
		{
		public:
      enum class ShadowType
      {
        NONE, SOFT, HARD
      };
      enum class Type
      {
        AMBIENT,
        DIRECTIONAL,
        HEMISPHERIC,
        POINT,
        SPOT
      };
			POMPEIIENGINE_API
			Light( Light::Type t = Light::Type::POINT );
			POMPEIIENGINE_API
			virtual ~Light( void );
			POMPEIIENGINE_API
    	inline const Light::Type& getType( void ) const 
    	{
    		return _type;
    	}
			POMPEIIENGINE_API
    	const Color& getColor( void ) 
    	{
    		return _diffuseColor;
    	}
			POMPEIIENGINE_API
    	void setColor( const Color& c )
    	{
    		_diffuseColor = c;
    	}
			POMPEIIENGINE_API
    	/*const Color&*/Color getGroundColor( void )
    	{
    		if ( _type == Type::HEMISPHERIC )
    		{
    			return _groundColor;
    		}
    		return Color( 0.0f, 0.0f, 0.0f );
    	}
			POMPEIIENGINE_API
    	void setGroundColor( const Color& c )
    	{
    		_groundColor = c;
    	}
      POMPEIIENGINE_API
      glm::mat4 computeProjectionMatrix( void) // TODO: const
      {
        return glm::ortho( 
          -10.0f, 10.0f, 
          -10.0f, 10.0f, 
          _shadowNear, _shadowFar
        );
      }
      POMPEIIENGINE_API
      glm::mat4 computeViewMatrix( void) // TODO: const
      {
        return glm::inverse( getTransform( ) );
      }
      POMPEIIENGINE_API
      glm::vec3 getPosition( void) // TODO: const
      {
        return getAbsolutePosition( );
      }
      POMPEIIENGINE_API
      glm::vec3 getDirection( void ) // TODO: const
      {
        if ( _type == Type::POINT )
        {
          return glm::zero<glm::vec3>( );
        }
        return glm::eulerAngles( getAbsoluteRotation( ) );
      }
    public:
    	POMPEIIENGINE_API
    	virtual void accept( Visitor& v ) override;
    protected:
    	Light::Type _type;

    	Color _diffuseColor;
    	Color _ambientColor;
    	Color _groundColor; // Only for hemispheric light

      Light::ShadowType _shadowType;

      float _shadowNear = 0.1f;
      float _shadowFar = 1024.0f;
    		
		  float Constant = 1.0f;   // default: 1
		  float Linear = 0.0f;     // default: 0
		  float Quadratic = 0.0f;  // default: 0
		  float Intensity = 1.0f;  // default: 1
		};
	}
}

#endif /* __POMPEII_ENGINE_LIGHT__ */