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

#ifndef __LAVARENDERER_KEYBOARD__
#define __LAVARENDERER_KEYBOARD__

#include <lavaRenderer/api.h>
#include <bitset>
#include "../includes.hpp"

namespace lava
{
  class Keyboard
  {
  public:
    enum class Key
    {
      A,
      B,
      C,
      D,
      E,
      F,
      G,
      H,
      I,
      J,
      K,
      L,
      M,
      N,
      O,
      P,
      Q,
      R,
      S,
      T,
      U,
      V,
      W,
      X,
      Y,
      Z,
      Up,
      Down,
      Left,
      Right,
      F1,
      F2,
      F3,
      F4,
      F5,
      F6,
      F7,
      F8,
      F9,
      F10,
      F11,
      F12,
      Num1,
      Num2,
      Num3,
      Num4,
      Num5,
      Num6,
      Num7,
      Num8,
      Num9,
      Num0,
      Esc,
      Enter,
      Space,
      LShift,
      RShift,
      LAlt,
      RAlt,
      Plus,
      Minus,
      Multiply,
      Divide
    };

    virtual ~Keyboard();

    virtual bool keyPress(const Key& k) = 0;
    virtual bool keyRelease(const Key& k) = 0;
    virtual bool singleKeyPress(const Key& k) = 0;
    virtual void update() = 0;
  };
  class GLFWKeyboard : public Keyboard
  {
  public:
    LAVARENDERER_API
    void onKeyUp( int code );
    LAVARENDERER_API
    void onKeyDown( int code );
    LAVARENDERER_API
    virtual bool keyPress( const Key& k );
    LAVARENDERER_API
    virtual bool keyRelease( const Key& k );
    LAVARENDERER_API
    virtual bool singleKeyPress( const Key& k );
    LAVARENDERER_API
    virtual void update( );
  private:
    std::bitset<GLFW_KEY_LAST> _keyPreviousState;
    std::bitset<GLFW_KEY_LAST> _isKeyPressed;
    std::bitset<GLFW_KEY_LAST> _isKeyClicked;

    int ConvertFromKey( Keyboard::Key k );
  };
}

#endif /* __LAVARENDERER_KEYBOARD__ */
