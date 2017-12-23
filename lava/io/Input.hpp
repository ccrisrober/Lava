/**
 * Copyright (c) 2017, Lava
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


#ifndef __LAVA_INPUT__
#define __LAVA_INPUT__

#include <lava/api.h>

#include "Mouse.hpp"
#include "Keyboard.hpp"
#include <unordered_map>

namespace lava
{
  class Input
  {
  public:
	LAVA_API
	static void initialize( void );
	LAVA_API
	static void destroy( void );
	LAVA_API
	static lava::Keyboard* Keyboard( void );
	LAVA_API
	static lava::Mouse* Mouse( void );
	LAVA_API
	static bool isKeyPressed(Keyboard::Key key);
	LAVA_API
	static bool isKeyClicked(Keyboard::Key key);
	LAVA_API
	static bool KeyReleased(Keyboard::Key key);
	LAVA_API
	static int MouseX( void );
	LAVA_API
	static int MouseY( void );
	LAVA_API
	static Input* instance( void );
	LAVA_API
	static int PreviousMouseX( void );
	LAVA_API
	static int PreviousMouseY( void );
	LAVA_API
	static int MouseWheelX( void );
	LAVA_API
	static int MouseWheelY( void );
	LAVA_API
	static int DeltaX( int val );
	LAVA_API
	static int DeltaY( int val );
	LAVA_API
	static bool MouseButtonPress( MouseButton button );
	LAVA_API
	static bool MouseButtonSinglePress( MouseButton button );
	LAVA_API
	static bool MouseButtonRelease( MouseButton button );
	LAVA_API
	static void update( void );

	LAVA_API
	static float getAxis( const std::string& axis )
	{
		return Input::instance( )->_axes[ axis ];
	}
	LAVA_API
	static void setAxis( const std::string& axis, float v )
	{
		Input::instance( )->_axes[ axis ] = v;
	}
  protected:
	lava::Keyboard* _keyboard;
	lava::Mouse* _mouse;

	std::unordered_map< std::string, float > _axes;

	const char* AXIS_HORIZONTAL = "Horizontal";
	const char* AXIS_VERTICAL = "Vertical";

	static Input *_instance;
  };
}

#endif /* __LAVA_INPUT__ */
