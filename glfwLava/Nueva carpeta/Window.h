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

#ifndef __LAVARENDERER_WINDOW__
#define __LAVARENDERER_WINDOW__

#include <glfwLava/api.h>
#include <functional>

#include "includes.hpp"

namespace lava
{
  class Window
  {
  public:
    GLFWLAVA_API
    Window( const char* title, uint32_t width, uint32_t height );
    GLFWLAVA_API
    ~Window( void );

    GLFWLAVA_API
    void setWindowTitle( const std::string& title );

    GLFWLAVA_API
    GLFWwindow *getWindow( void );

    GLFWLAVA_API
    void pollEvents( void );

    GLFWLAVA_API
    inline operator GLFWwindow*( void )
    {
      return _window;
    }

    GLFWLAVA_API
    void setChangeSizeCallback( std::function<void( int, int )> cb );

    GLFWLAVA_API
    void setErrorCallback( GLFWerrorfun fn );

    GLFWLAVA_API
    uint32_t getWidth( void ) const;
    GLFWLAVA_API
    uint32_t getHeight( void ) const;

    GLFWLAVA_API
    bool isRunning( void ) const;

    GLFWLAVA_API
    void close( void );
  protected:
    GLFWwindow* _window;
  public:
    std::function<void( int, int )> _callbackResize = nullptr;
  };
}

#endif /* __LAVARENDERER_WINDOW__ */