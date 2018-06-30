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

#ifndef __GLFWLAVA_INPUT__
#define __GLFWLAVA_INPUT__

#include <glfwLava/api.h>

#include "Mouse.hpp"
#include "Keyboard.hpp"
#include <unordered_map>

namespace lava
{
  class Input
  {
  public:
	GLFWLAVA_API
	static void initialize( void );
	GLFWLAVA_API
	static void destroy( void );
	GLFWLAVA_API
	static lava::Keyboard* Keyboard( void );
	GLFWLAVA_API
	static lava::Mouse* Mouse( void );
	GLFWLAVA_API
	static bool isKeyPressed(Keyboard::Key key);
	GLFWLAVA_API
	static bool isKeyClicked(Keyboard::Key key);
	GLFWLAVA_API
	static bool KeyReleased(Keyboard::Key key);
	GLFWLAVA_API
	static int MouseX( void );
	GLFWLAVA_API
	static int MouseY( void );
	GLFWLAVA_API
	static Input* instance( void );
	GLFWLAVA_API
	static int PreviousMouseX( void );
	GLFWLAVA_API
	static int PreviousMouseY( void );
	GLFWLAVA_API
	static int MouseWheelX( void );
	GLFWLAVA_API
	static int MouseWheelY( void );
	GLFWLAVA_API
	static int DeltaX( int val );
	GLFWLAVA_API
	static int DeltaY( int val );
	GLFWLAVA_API
	static bool MouseButtonPress( MouseButton button );
	GLFWLAVA_API
	static bool MouseButtonSinglePress( MouseButton button );
	GLFWLAVA_API
	static bool MouseButtonRelease( MouseButton button );
	GLFWLAVA_API
	static void update( void );

	GLFWLAVA_API
	static float getAxis( const std::string& axis )
	{
		return Input::instance( )->_axes[ axis ];
	}
	GLFWLAVA_API
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

#endif /* __GLFWLAVA_INPUT__ */
