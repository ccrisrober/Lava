#ifndef __LAVAENGINE_COLOR__
#define __LAVAENGINE_COLOR__

#include "Mathf.h"

namespace lava
{
  namespace engine
  {
    class Color
    {
    public:
      LAVAENGINE_API
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

      LAVAENGINE_API
      Color linear( void );
      LAVAENGINE_API
      Color gamma( void );
      LAVAENGINE_API
      float grayscale( void ) const;
      LAVAENGINE_API
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
      LAVAENGINE_API
      static Color lerp( const Color& a, const Color& b, float t );
      LAVAENGINE_API
      static Color LerpUnclamped( const Color& a, const Color& b, float t );
      LAVAENGINE_API
      Color& RGBMultiplied( float multiplier );
      LAVAENGINE_API
      Color& AlphaMultiplied( float multiplier );
      LAVAENGINE_API
      Color& RGBMultiplied( const Color& multiplier );
      LAVAENGINE_API
      static void RGBToHSV( const Color& rgbColor, float& h, float& s, float& v );
    private:
      static void RGBToHSVHelper( float offset, float dominantcolor,
        float colorone, float colortwo, float& H, float& S, float& V );
    public:
      LAVAENGINE_API
      static Color HSVToRGB( float H, float S, float V );
      LAVAENGINE_API
      static Color HSVToRGB( float H, float S, float V, bool hdr );
      LAVAENGINE_API
      static Color createFromHex( int hex );

    public:
      LAVAENGINE_API
      const static Color AQUA;
      LAVAENGINE_API
      const static Color BEIGE;
      LAVAENGINE_API
      const static Color BLACK;
      LAVAENGINE_API
      const static Color BLUE;
      LAVAENGINE_API
      const static Color BROWN;
      LAVAENGINE_API
      const static Color CYAN;
      LAVAENGINE_API
      const static Color GOLD;
      LAVAENGINE_API
      const static Color GREEN;
      LAVAENGINE_API
      const static Color GREY;
      LAVAENGINE_API
      const static Color INDIGO;
      LAVAENGINE_API
      const static Color LAVENDER;
      LAVAENGINE_API
      const static Color ORANGE;
      LAVAENGINE_API
      const static Color PINK;
      LAVAENGINE_API
      const static Color PURPLE;
      LAVAENGINE_API
      const static Color RED;
      LAVAENGINE_API
      const static Color YELLOW;
      LAVAENGINE_API
      const static Color WHITE;
    protected:
      float _r, _g, _b, _a;
    };
  }
}

#endif /* __LAVAENGINE_COLOR__ */
