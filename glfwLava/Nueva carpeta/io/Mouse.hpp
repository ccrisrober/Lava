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

#ifndef __LAVARENDERER_MOUSE__
#define __LAVARENDERER_MOUSE__

#include <glfwLava/api.h>
#include <cstring>
#include "../includes.hpp"

namespace lava
{
  enum MouseButton
  {
    Left = 0,
    Middle = 1,
    Right = 2
  };
  class Mouse
  {
  public:
    virtual ~Mouse( void );
    virtual int mouseX( void ) = 0;
    virtual int mouseY( void ) = 0;
    virtual int previousMouseX( void ) = 0;
    virtual int previousMouseY( void ) = 0;
    virtual int mouseWheelX( void ) = 0;
    virtual int mouseWheelY( void ) = 0;
    virtual int deltaX( int val ) = 0;
    virtual int deltaY( int val ) = 0;
    virtual bool buttonRelease( MouseButton button ) = 0;
    virtual bool buttonPress( MouseButton button ) = 0;
    virtual bool singleButtonPress( MouseButton button ) = 0;
    virtual void update( void ) = 0;
  };
  class GLFWMouse : public Mouse
  {
  public:
    GLFWLAVA_API
    GLFWMouse( void );
    GLFWLAVA_API
    virtual ~GLFWMouse( void );
    GLFWLAVA_API
    virtual int mouseX( void );
    GLFWLAVA_API
    virtual int mouseY( void );
    GLFWLAVA_API
    virtual int previousMouseX( void );
    GLFWLAVA_API
    virtual int previousMouseY( void );
    GLFWLAVA_API
    virtual int mouseWheelX( void );
    GLFWLAVA_API
    virtual int mouseWheelY( void );
    GLFWLAVA_API
    virtual int deltaX( int val );
    GLFWLAVA_API
    virtual int deltaY( int val );
    GLFWLAVA_API
    virtual bool buttonRelease( MouseButton button );
    GLFWLAVA_API
    virtual bool buttonPress( MouseButton button );
    GLFWLAVA_API
    virtual bool singleButtonPress( MouseButton button );
    GLFWLAVA_API
    virtual void update( void );

    void onMouseMove( int x, int y );
    void onMouseButtonEvent( int e, int action );
    void onMouseWheelEvent( int xoffset, int yoffset );
    struct GLFWMouseState
    {
      short clicks = 0;
      short state;
    };
  protected:
    int _x;
    int _y;
    int _prevX;
    int _prevY;
    int _wheelX;
    int _wheelY;
    int _prevWheelX;
    int _prevWheelY;
    GLFWMouseState _current[ 3 ];
    GLFWMouseState _previous[ 3 ];
  };
}

#endif /* __LAVARENDERER_MOUSE__ */
