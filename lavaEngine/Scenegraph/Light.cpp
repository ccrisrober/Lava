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

#include "Light.h"
#include <iostream>

namespace lava
{
	namespace engine
	{
		Light::Light( Light::Type t )
			: Node( std::string( "Light" ) )
			, _type( t )
			, _diffuseColor( Color( 1.0f, 1.0f, 1.0f ) )
			, _ambientColor( Color( 0.0f, 0.0f, 0.0f ) )
			, _groundColor( Color( 0.0f, 0.0f, 0.0f ) )
      , _shadowType( ShadowType::NONE )
		{
		}
		Light::~Light( void )
		{
    		std::cout << "[D] Light '" << this->name( ) << "'" << std::endl;
		}

		void Light::accept( Visitor& visitor )
		{
			visitor.visitLight( this );
		}
	}
}