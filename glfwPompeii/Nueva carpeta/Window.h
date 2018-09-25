/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#ifndef __GLFWPOMPEII_WINDOW__
#define __GLFWPOMPEII_WINDOW__

#include <glfwPompeii/api.h>
#include <functional>

#include "includes.hpp"

namespace pompeii
{
  class Window
  {
  public:
    GLFWPOMPEII_API
    Window( const char* title, uint32_t width, uint32_t height );
    GLFWPOMPEII_API
    ~Window( void );

    GLFWPOMPEII_API
    void setWindowTitle( const std::string& title );

    GLFWPOMPEII_API
    GLFWwindow *getWindow( void );

    GLFWPOMPEII_API
    void pollEvents( void );

    GLFWPOMPEII_API
    inline operator GLFWwindow*( void )
    {
      return _window;
    }

    GLFWPOMPEII_API
    void setChangeSizeCallback( std::function<void( int, int )> cb );

    GLFWPOMPEII_API
    void setErrorCallback( GLFWerrorfun fn );

    GLFWPOMPEII_API
    uint32_t getWidth( void ) const;
    GLFWPOMPEII_API
    uint32_t getHeight( void ) const;

    GLFWPOMPEII_API
    bool isRunning( void ) const;

    GLFWPOMPEII_API
    void close( void );
  protected:
    GLFWwindow* _window;
  public:
    std::function<void( int, int )> _callbackResize = nullptr;
  };
}

#endif /* __GLFWPOMPEII_WINDOW__ */