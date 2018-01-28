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

#include "Mouse.hpp"

namespace lava
{
  Mouse::~Mouse( void ) { }
  GLFWMouse::GLFWMouse( void )
    : Mouse( )
    , _x( 0 )
    , _y( 0 )
    , _prevX( 0 )
    , _prevY( 0 )
    , _wheelX( 0 )
    , _wheelY( 0 )
    , _prevWheelX( 0 )
    , _prevWheelY( 0 )
  {
    _current[ MouseButton::Left ].state = GLFW_RELEASE;
    _current[ MouseButton::Middle ].state = GLFW_RELEASE;
    _current[ MouseButton::Right ].state = GLFW_RELEASE;

    _previous[ MouseButton::Left ].state = GLFW_RELEASE;
    _previous[ MouseButton::Middle ].state = GLFW_RELEASE;
    _previous[ MouseButton::Right ].state = GLFW_RELEASE;
  }
  GLFWMouse::~GLFWMouse( void ) { }
  int GLFWMouse::mouseX( void )
  {
    return _x;
  }
  int GLFWMouse::mouseY( void )
  {
    return _y;
  }
  int GLFWMouse::previousMouseX( void )
  {
    return _prevX;
  }
  int GLFWMouse::previousMouseY( void )
  {
    return _prevY;
  }
  int GLFWMouse::mouseWheelX( void )
  {
    return _wheelX;
  }
  int GLFWMouse::mouseWheelY( void )
  {
    return _wheelY;
  }
  int GLFWMouse::deltaX( int val )
  {
    if (_x != _prevX)
    {
      return _x - val;
    }
    return 0;
  }
  int GLFWMouse::deltaY( int val )
  {
    if (_y != _prevY)
    {
      return _y - val;
    }
    return 0;
  }
  bool GLFWMouse::buttonRelease( MouseButton button )
  {
    switch (button)
    {
    case MouseButton::Left:
      return _current[ MouseButton::Left ].state == GLFW_RELEASE && _previous[ MouseButton::Left ].state == GLFW_PRESS;
    case MouseButton::Middle:
      return _current[ MouseButton::Middle].state == GLFW_RELEASE && _previous[ MouseButton::Middle].state == GLFW_PRESS;
    case MouseButton::Right:
      return _current[ MouseButton::Right].state == GLFW_RELEASE && _previous[ MouseButton::Right].state == GLFW_PRESS;
    default:
      return false;
    }
  }
  bool GLFWMouse::buttonPress( MouseButton button )
  {
    switch (button)
    {
    case MouseButton::Left:
      return _current[ MouseButton::Left ].state == GLFW_PRESS;
    case MouseButton::Middle:
      return _current[ MouseButton::Middle].state == GLFW_PRESS;
    case MouseButton::Right:
      return _current[ MouseButton::Right].state == GLFW_PRESS;
    default:
      return false;
    }
  }
  bool GLFWMouse::singleButtonPress( MouseButton button )
  {
    switch (button)
    {
    case MouseButton::Left:
      return _current[ MouseButton::Left ].state == GLFW_PRESS && _previous[ MouseButton::Left ].state == GLFW_RELEASE;
    case MouseButton::Middle:
      return _current[ MouseButton::Middle].state == GLFW_PRESS && _previous[ MouseButton::Middle].state == GLFW_RELEASE;
    case MouseButton::Right:
      return _current[ MouseButton::Right].state == GLFW_PRESS && _previous[ MouseButton::Right].state == GLFW_RELEASE;
    default:
      return false;
    }
  }
  void GLFWMouse::update( void )
  {
    _prevWheelX = _wheelX;
    _prevWheelY = _wheelY;
    _prevX = _x;
    _prevY = _y;
    _wheelY = 0;
    std::memcpy(_previous, _current, sizeof(_current));
  }

  void GLFWMouse::onMouseMove( int x, int y )
  {
    _x = x;
    _y = y;
  }

  void GLFWMouse::onMouseButtonEvent( int e, int action )
  {
    switch (e)
    {
      case GLFW_MOUSE_BUTTON_LEFT:
        _current[ MouseButton::Left ].state = static_cast<uint8_t>(action);
        _current[ MouseButton::Left ].clicks = 1;
        break;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        _current[ MouseButton::Middle].state = static_cast<uint8_t>(action);
        _current[ MouseButton::Middle].clicks = 1;
        break;
      case GLFW_MOUSE_BUTTON_RIGHT:
        _current[ MouseButton::Right].state = static_cast<uint8_t>(action);
        _current[ MouseButton::Right].clicks = 1;
      break;
    }
  }

  void GLFWMouse::onMouseWheelEvent( int xoffset, int yoffset )
  {
    _wheelX = xoffset;
    _wheelY = yoffset;
  }
}
