#ifndef __LAVAENGINE_MATHF__
#define __LAVAENGINE_MATHF__

#include <lavaEngine/api.h>

#include <cmath>
#include <algorithm>
#include <string>

namespace lava
{
	class Mathf
	{
	public:
    /**
    * Linear interpolation.
    * @param  {float} x   [description]
    * @param  {float} x1  [description]
    * @param  {float} x2  [description]
    * @param  {float} q00 [description]
    * @param  {float} q01 [description]
    * @return {float}     [description]
    */
    LAVAENGINE_API
    static float lerp(float x, float x1, float x2, float q00, float q01);
    /**
    * Bilinear interpolation
    * @param  {number} x   [description]
    * @param  {number} y   [description]
    * @param  {number} q11 [description]
    * @param  {number} q12 [description]
    * @param  {number} q21 [description]
    * @param  {number} q22 [description]
    * @param  {number} x1  [description]
    * @param  {number} x2  [description]
    * @param  {number} y1  [description]
    * @param  {number} y2  [description]
    * @return {number}     [description]
    */
    LAVAENGINE_API
    static float biLerp(float x, float y, float q11, float q12, float q21,
      float q22, float x1, float x2, float y1, float y2);
    /**
    * Trilinear interpolation.
    * @param  {number} x    [description]
    * @param  {number} y    [description]
    * @param  {number} z    [description]
    * @param  {number} q000 [description]
    * @param  {number} q001 [description]
    * @param  {number} q010 [description]
    * @param  {number} q011 [description]
    * @param  {number} q100 [description]
    * @param  {number} q101 [description]
    * @param  {number} q110 [description]
    * @param  {number} q111 [description]
    * @param  {number} x1   [description]
    * @param  {number} x2   [description]
    * @param  {number} y1   [description]
    * @param  {number} y2   [description]
    * @param  {number} z1   [description]
    * @param  {number} z2   [description]
    * @return {number}      [description]
    */
    LAVAENGINE_API
    static float triLerp(float x, float y, float z, float q000,
        float q001, float q010, float q011, float q100, float q101,
        float q110, float q111, float x1, float x2, float y1, float y2,
        float z1, float z2);
    /**
    * Converts degrees angle to radians angle.
    * @param  {number} degs Degrees angle
    * @return {number}      Radians angle
    */
    LAVAENGINE_API
    static float degToRad(float degs);
    /**
    * Converts radians angle to degrees angle.
    * @param  {number} degs Radians angle
    * @return {number}      Degrees angle
    */
    LAVAENGINE_API
    static float radToDeg(float rads);
    /**
    * Returns true if the value is power of two.
    * @param  {number} v Integer value.
    * @return {boolean}
    */
    LAVAENGINE_API
    static bool isPOT(unsigned int v);
    /**
    * Returns the next power of two value.
    * @param  {number} v Integer value.
    * @return {number}
    */
    LAVAENGINE_API
    static unsigned int nearestPOT(unsigned int v);
    /**
    * Clamps a value to be between a minimum and maximum value.
    * @param  {number} v   Value to clamp.
    * @param  {number} min Minimum value.
    * @param  {number} max Maximum value
    * @return {number}
    */
    LAVAENGINE_API
    static float clamp(float v, float min, float max);
    /**
    * Clamps value between 0 and 1 and returns value.
    * @param  {number} v Value to clamp.
    * @return {number}
    */
    LAVAENGINE_API
    static float clamp01(float v);
    /**
    * Return 1 when is a positive number. -1 otherwise.
    * @param  {number} v [description]
    * @return {number}   [description]
    */
    LAVAENGINE_API
    static int sign(int v);
    /**
    * Normalizes radians angle between [0, 2π].
    * @param  {number} radAngle Radian angle.
    * @return {number}          Normalizated radian angle.
    */
    LAVAENGINE_API
    static float normalizeAngle(float radAngle);
    /**
    * Interpolates between min and max with smoothing at the limits.
    * @param  {number}     x   Value to interpolate.
    * @param  {number = 0} min Minimum value.
    * @param  {number = 1} max Maximum value.
    * @return {number}         Interpolated value
    */
    LAVAENGINE_API
    static float smoothstep(float x, float min = 0.0f, float max = 1.0f);
    /**
    * Interpolates between min and max with more smoothing at the limits thatn smoothstep.
    * @param  {number}     x   Value to interpolate.
    * @param  {number = 0} min Minimum value.
    * @param  {number = 1} max Maximum value.
    * @return {number}         Interpolated value
    */
    LAVAENGINE_API
    static float smootherstep(float x, float min, float max);
    /**
    * Convert number to hexadecimal.
    * @param  {number} n Number value.
    * @return {string}   Hexadecimal representation.
    */
    LAVAENGINE_API
    static std::string toHex( int n );

    /**
    * Convert current color from gamma to linear range.
    */
    LAVAENGINE_API
    static float gammaToLinearSpace( const float& v,
      const float& gammaFactor = 2.2f )
    {
      return std::pow( v, gammaFactor );
    }
    /**
    * Convert current color from linear to gamma range.
    */
    LAVAENGINE_API
    static float linearToGammaSpace( const float& v,
      const float& gammaFactor = 2.2f )
    {
      float invGamma = ( gammaFactor > 0.0f ) ? ( 1.0f / gammaFactor ) : 1.0f;
      return std::pow( v, invGamma );
    }

    LAVAENGINE_API
    static bool approximately( float a, float b )
    {
      return std::fabs(a - b) < 0.00001f;
    }

    // compute euclidian modulo of m % n
    // https://en.wikipedia.org/wiki/Modulo_operation
    LAVAENGINE_API
    static unsigned int euclideanModulo( unsigned int m, unsigned int n )
    {
      return ( ( n % m ) + m ) % m;
    }
    LAVAENGINE_API
    static const float PI_2;
    LAVAENGINE_API
    static const float PI;
    LAVAENGINE_API
    static const float TWO_PI;
    LAVAENGINE_API
    static const float Deg2Rad;
    LAVAENGINE_API
    static const float Rad2Deg;

    /*LAVAENGINE_API
    static float distance( const mb::Vector3& u, const mb::Vector3& v )
    {
      return std::sqrt( distanceSq( u, v ) );
    }
    LAVAENGINE_API
    static float distanceSq( const mb::Vector3& u, const mb::Vector3& v )
    {
      return ( v - u ).getSquaredMagnitude( );
    }
    LAVAENGINE_API
    static float distance( const mb::Ray& r, const mb::Vector3& p )
    {
      return std::sqrt( distanceSq( r, p ) );
    }
    LAVAENGINE_API
    static float distanceSq( const mb::Ray& r, const mb::Vector3& p )
    {
      mb::Vector3 v0 = p - r.getOrigin( );
      float v1 = v0 * r.getDirection( );
      return ( v0 * v0 - v1 * v1 / ( r.getDirection( ).getSquaredMagnitude( ) ) );
    }*/
	};
}

#endif /* __LAVAENGINE_MATHF__ */
