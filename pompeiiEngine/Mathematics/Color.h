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

#ifndef __POMPEIIENGINE_COLOR__
#define __POMPEIIENGINE_COLOR__

#include "Mathf.h"

namespace pompeii
{
  namespace engine
  {
    class Color
    {
    public:
      POMPEIIENGINE_API
      Color( float r_, float g_, float b_, float a_ = 1.0f )
        : _r( r_ )
        , _g( g_ )
        , _b( b_ )
        , _a( a_ )
      {
      }

      static Color randomColor( void )
      {
        return Color(
          static_cast < float > ( rand( ) ) / static_cast < float > ( RAND_MAX ),
          static_cast < float > ( rand( ) ) / static_cast < float > ( RAND_MAX ),
          static_cast < float > ( rand( ) ) / static_cast < float > ( RAND_MAX ),
          1.0f
        );
        // TODO: USE BETTER RANDOM METHODS ...
      }

      float r( void ) const { return _r; }
      float g( void ) const { return _g; }
      float b( void ) const { return _b; }
      float a( void ) const { return _a; }
      float& r( void ) { return _r; }
      float& g( void ) { return _g; }
      float& b( void ) { return _b; }
      float& a( void ) { return _a; }

      POMPEIIENGINE_API
      Color linear( void );
      POMPEIIENGINE_API
      Color gamma( void );
      POMPEIIENGINE_API
      float grayscale( void ) const;
      POMPEIIENGINE_API
      float maxColorComponent( void ) const;
      friend Color operator+( Color& a, const Color& b )
      {
        return Color(
          a.r( ) + b.r( ),
          a.g( ) + b.g( ),
          a.b( ) + b.b( ),
          a.a( ) + b.a( )
        );
      }
      friend Color& operator+=( Color& a, const Color& b )
      {
        a.r( ) += b.r( );
        a.g( ) += b.g( );
        a.b( ) += b.b( );
        a.a( ) += b.a( );
        return a;
      }
      friend Color operator-( Color& a, const Color& b )
      {
        return Color(
          a.r( ) - b.r( ),
          a.g( ) - b.g( ),
          a.b( ) - b.b( ),
          a.a( ) - b.a( )
        );
      }
      friend Color& operator-=( Color& a, const Color& b )
      {
        a.r( ) -= b.r( );
        a.g( ) -= b.g( );
        a.b( ) -= b.b( );
        a.a( ) -= b.a( );
        return a;
      }
      friend Color operator*( Color& a, const Color& b )
      {
        return Color(
          a.r( ) * b.r( ),
          a.g( ) * b.g( ),
          a.b( ) * b.b( ),
          a.a( ) * b.a( )
        );
      }
      friend Color& operator*=( Color& a, const Color& b )
      {
        a.r( ) *= b.r( );
        a.g( ) *= b.g( );
        a.b( ) *= b.b( );
        a.a( ) *= b.a( );
        return a;
      }
      friend Color operator/( Color& a, const Color& b )
      {
        return Color(
          a.r( ) / b.r( ),
          a.g( ) / b.g( ),
          a.b( ) / b.b( ),
          a.a( ) / b.a( )
        );
      }
      friend Color& operator/=( Color& a, const Color& b )
      {
        a.r( ) /= b.r( );
        a.g( ) /= b.g( );
        a.b( ) /= b.b( );
        a.a( ) /= b.a( );
        return a;
      }
      bool operator==( const Color& c ) const
      {
        return
          ( this->r( ) == c.r( ) ) &&
          ( this->g( ) == c.g( ) ) &&
          ( this->b( ) == c.b( ) ) &&
          ( this->a( ) == c.a( ) );
      }
      bool operator !=( const Color& c ) const
      {
        return !( *this == c );
      }
      POMPEIIENGINE_API
      static Color lerp( const Color& a, const Color& b, float t );
      POMPEIIENGINE_API
      static Color LerpUnclamped( const Color& a, const Color& b, float t );
      POMPEIIENGINE_API
      Color& RGBMultiplied( float multiplier );
      POMPEIIENGINE_API
      Color& AlphaMultiplied( float multiplier );
      POMPEIIENGINE_API
      Color& RGBMultiplied( const Color& multiplier );
      POMPEIIENGINE_API
      static void RGBToHSV( const Color& rgbColor, float& h, float& s, float& v );
    private:
      static void RGBToHSVHelper( float offset, float dominantcolor,
        float colorone, float colortwo, float& H, float& S, float& V );
    public:
      POMPEIIENGINE_API
      static Color HSVToRGB( float H, float S, float V );
      POMPEIIENGINE_API
      static Color HSVToRGB( float H, float S, float V, bool hdr );
      POMPEIIENGINE_API
      static Color createFromHex( int hex );

    public:
      POMPEIIENGINE_API
      const static Color AQUA;
      POMPEIIENGINE_API
      const static Color BEIGE;
      POMPEIIENGINE_API
      const static Color BLACK;
      POMPEIIENGINE_API
      const static Color BLUE;
      POMPEIIENGINE_API
      const static Color BROWN;
      POMPEIIENGINE_API
      const static Color CYAN;
      POMPEIIENGINE_API
      const static Color GOLD;
      POMPEIIENGINE_API
      const static Color GREEN;
      POMPEIIENGINE_API
      const static Color GREY;
      POMPEIIENGINE_API
      const static Color INDIGO;
      POMPEIIENGINE_API
      const static Color LAVENDER;
      POMPEIIENGINE_API
      const static Color ORANGE;
      POMPEIIENGINE_API
      const static Color PINK;
      POMPEIIENGINE_API
      const static Color PURPLE;
      POMPEIIENGINE_API
      const static Color RED;
      POMPEIIENGINE_API
      const static Color YELLOW;
      POMPEIIENGINE_API
      const static Color WHITE;
    protected:
      float _r, _g, _b, _a;
    };
  }
}

#endif /* __POMPEIIENGINE_COLOR__ */
