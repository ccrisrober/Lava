/**
 * Copyright (c) 2017, Monkey Brush
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

#include "Color.h"

namespace pompeii
{
  namespace engine
  {
    const Color Color::AQUA = Color::createFromHex( 0x00FFFF );
    const Color Color::BEIGE = Color::createFromHex( 0xF5F5DC );
    const Color Color::BLACK = Color::createFromHex( 0x000000 );
    const Color Color::BLUE = Color::createFromHex( 0x0000FF );
    const Color Color::BROWN = Color::createFromHex( 0xA52A2A );
    const Color Color::CYAN = Color::createFromHex( 0x00FFFF );
    const Color Color::GOLD = Color::createFromHex( 0xFFD700 );
    const Color Color::GREEN = Color::createFromHex( 0x008000 );
    const Color Color::GREY = Color::createFromHex( 0x808080 );
    const Color Color::INDIGO = Color::createFromHex( 0x4B0082 );
    const Color Color::LAVENDER = Color::createFromHex( 0xE6E6FA );
    const Color Color::ORANGE = Color::createFromHex( 0xFFA500 );
    const Color Color::PINK = Color::createFromHex( 0xFFC0CB );
    const Color Color::PURPLE = Color::createFromHex( 0x800080 );
    const Color Color::RED = Color::createFromHex( 0xFF0000 );
    const Color Color::YELLOW = Color::createFromHex( 0xFFFF00 );
    const Color Color::WHITE = Color::createFromHex( 0xFFFFFF );

    Color Color::linear( void )
    {
      return Color(
        Mathf::gammaToLinearSpace( r( ) ),
        Mathf::gammaToLinearSpace( g( ) ),
        Mathf::gammaToLinearSpace( b( ) ), a( ) );
    }
    Color Color::gamma( void )
    {
      return Color(
        Mathf::linearToGammaSpace( r( ) ),
        Mathf::linearToGammaSpace( g( ) ),
        Mathf::linearToGammaSpace( b( ) ), a( ) );
    }
    float Color::grayscale( void ) const
    {
      return 0.299f * this->r( ) + 0.587f * this->g( ) + 0.114f * this->b( );
    }
    float Color::maxColorComponent( void ) const
    {
      return std::max( std::max( r( ), g( ) ), a( ) );
    }

    Color Color::lerp( const Color& a, const Color& b, float t )
    {
      t = Mathf::clamp01( t );
      return Color(
        a.r( ) + ( b.r( ) - a.r( ) ) * t,
        a.g( ) + ( b.g( ) - a.g( ) ) * t,
        a.b( ) + ( b.b( ) - a.b( ) ) * t,
        a.a( ) + ( b.a( ) - a.a( ) ) * t
      );
    }
    Color Color::LerpUnclamped( const Color& a, const Color& b, float t )
    {
      return Color(
        a.r( ) + ( b.r( ) - a.r( ) ) * t,
        a.g( ) + ( b.g( ) - a.g( ) ) * t,
        a.b( ) + ( b.b( ) - a.b( ) ) * t,
        a.a( ) + ( b.a( ) - a.a( ) ) * t
      );
    }
    Color& Color::RGBMultiplied( float multiplier )
    {
      this->r( ) *= multiplier;
      this->g( ) *= multiplier;
      this->b( ) *= multiplier;
      return *this;
    }
    Color& Color::AlphaMultiplied( float multiplier )
    {
      this->a( ) *= multiplier;
      return *this;
    }

    Color& Color::RGBMultiplied( const Color& multiplier )
    {
      this->r( ) *= multiplier.r( );
      this->g( ) *= multiplier.g( );
      this->b( ) *= multiplier.b( );
      return *this;
    }

    void Color::RGBToHSV( const Color& rgbColor, float& h, float& s, float& v )
    {
      if ( rgbColor.b( ) > rgbColor.g( ) && rgbColor.b( ) > rgbColor.r( ) )
      {
        Color::RGBToHSVHelper( 4.0f, rgbColor.b( ), rgbColor.r( ), rgbColor.g( ), h, s, v );
      }
      else if ( rgbColor.g( ) > rgbColor.r( ) )
      {
        Color::RGBToHSVHelper( 2.0f, rgbColor.g( ), rgbColor.b( ), rgbColor.r( ), h, s, v );
      }
      else
      {
        Color::RGBToHSVHelper( 0.0f, rgbColor.r( ), rgbColor.g( ), rgbColor.b( ), h, s, v );
      }
    }


    void Color::RGBToHSVHelper( float offset, float dominantcolor, float colorone, float colortwo, float& H, float& S, float& V )
    {
      V = dominantcolor;
      if ( V != 0.0f )
      {
        float num;
        if ( colorone > colortwo )
        {
          num = colortwo;
        }
        else
        {
          num = colorone;
        }
        float num2 = V - num;
        if ( num2 != 0.0f )
        {
          S = num2 / V;
          H = offset + ( colorone - colortwo ) / num2;
        }
        else
        {
          S = 0.0f;
          H = offset + ( colorone - colortwo );
        }
        H /= 6.0f;
        if ( H < 0.0f )
        {
          H += 1.0f;
        }
      }
      else
      {
        S = 0.0f;
        H = 0.0f;
      }
    }

    Color Color::HSVToRGB( float H, float S, float V )
    {
      return Color::HSVToRGB( H, S, V, true );
    }
    Color Color::HSVToRGB( float H, float S, float V, bool hdr )
    {
      Color white = Color::WHITE;
      if ( S == 0.0f )
      {
        white.r( ) = V;
        white.g( ) = V;
        white.b( ) = V;
      }
      else if ( V == 0.0f )
      {
        white.r( ) = 0.0f;
        white.g( ) = 0.0f;
        white.b( ) = 0.0f;
      }
      else
      {
        white.r( ) = 0.0f;
        white.g( ) = 0.0f;
        white.b( ) = 0.0f;
        float num = H * 6.0f;
        int num2 = ( int ) std::floor( num );
        float num3 = num - ( float ) num2;
        float num4 = V * ( 1.0f - S );
        float num5 = V * ( 1.0f - S * num3 );
        float num6 = V * ( 1.0f - S * ( 1.0f - num3 ) );
        switch ( num2 + 1 )
        {
        case 0:
          white.r( ) = V;
          white.g( ) = num4;
          white.b( ) = num5;
          break;
        case 1:
          white.r( ) = V;
          white.g( ) = num6;
          white.b( ) = num4;
          break;
        case 2:
          white.r( ) = num5;
          white.g( ) = V;
          white.b( ) = num4;
          break;
        case 3:
          white.r( ) = num4;
          white.g( ) = V;
          white.b( ) = num6;
          break;
        case 4:
          white.r( ) = num4;
          white.g( ) = num5;
          white.b( ) = V;
          break;
        case 5:
          white.r( ) = num6;
          white.g( ) = num4;
          white.b( ) = V;
          break;
        case 6:
          white.r( ) = V;
          white.g( ) = num4;
          white.b( ) = num5;
          break;
        case 7:
          white.r( ) = V;
          white.g( ) = num6;
          white.b( ) = num4;
          break;
        }
        if ( !hdr )
        {
          white.r( ) = Mathf::clamp( white.r( ), 0.0f, 1.0f );
          white.g( ) = Mathf::clamp( white.g( ), 0.0f, 1.0f );
          white.b( ) = Mathf::clamp( white.b( ), 0.0f, 1.0f );
        }
      }
      return white;
    }
    Color Color::createFromHex( int hex )
    {
      return Color(
        ( float ) ( hex >> 16 & 255 ) / 255.0f,
        ( float ) ( hex >> 8 & 255 ) / 255.0f,
        ( float ) ( hex & 255 ) / 255.0f
      );
    }
  }
}
