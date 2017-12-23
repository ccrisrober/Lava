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

#include "Window.h"

#include "io/Input.hpp"

namespace lava
{
  Window::Window( const char* title, uint32_t width, uint32_t height )
  {
    glfwInit( );
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    _window = glfwCreateWindow( width, height, title, nullptr, nullptr );

    glfwSetKeyCallback( _window, []( GLFWwindow*, int key, int, int act, int )
    {
      if ( act == GLFW_PRESS )
      {
        static_cast<GLFWKeyboard*>( Input::Keyboard( ) )->onKeyDown( key );
      }
      else if ( act == GLFW_RELEASE )
      {
        static_cast<GLFWKeyboard*>( Input::Keyboard( ) )->onKeyUp( key );
      }
    } );

    glfwSetCursorPosCallback( _window, []( GLFWwindow*, double xpos, double ypos )
    {
      static_cast<GLFWMouse*>( Input::Mouse( ) )->onMouseMove( static_cast<int>( xpos ), static_cast<int>( ypos ) );
    } );

    glfwSetMouseButtonCallback( _window, []( GLFWwindow*, int btn, int act, int )
    {
      static_cast<GLFWMouse*>( Input::Mouse( ) )->onMouseButtonEvent( btn, act );
    } );

    glfwSetScrollCallback( _window, []( GLFWwindow*, double xoffset, double yoffset )
    {
      static_cast<GLFWMouse*>( Input::Mouse( ) )->onMouseWheelEvent( static_cast<int>( xoffset ), static_cast<int>( yoffset ) );
    } );

    glfwSetWindowSizeCallback( _window, ( []( GLFWwindow* /*window*/, int width, int height )
    {
      // glViewport( 0, 0, width, height );
      // TODO: Send event and set width and height from _params
    } ) );

    glfwSetInputMode( _window, GLFW_STICKY_KEYS, GLFW_CURSOR_DISABLED );
    
    glfwMakeContextCurrent( _window );

    Input::initialize( );
  }

  GLFWwindow * Window::getWindow( void )
  {
    return _window;
  }

  void Window::pollEvents( void )
  {
    glfwPollEvents( );
  }

  void Window::setErrorCallback( GLFWerrorfun fn )
  {
    glfwSetErrorCallback( fn );
  }

  uint32_t Window::getWidth( void ) const
  {
    int width, height;
    glfwGetFramebufferSize( _window, &width, &height );
    return width;
  }

  uint32_t Window::getHeight( void ) const
  {
    int width, height;
    glfwGetFramebufferSize( _window, &width, &height );
    return height;
  }

  bool Window::isRunning( void ) const
  {
    //glfwPollEvents( );
    return !glfwWindowShouldClose( _window );
  }

  void Window::close( void )
  {
    glfwSetWindowShouldClose( _window, GLFW_TRUE );
  }

  Window::~Window( void )
  {
    glfwDestroyWindow( _window );
    glfwTerminate( );
  }

  void Window::setWindowTitle( const std::string& title )
  {
    glfwSetWindowTitle( _window, title.c_str( ) );
  }
}