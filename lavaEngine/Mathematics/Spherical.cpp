#include "Spherical.h"
#include "Mathf.h"
#include <float.h>

namespace lava
{
  namespace engine
  {
    glm::vec3 Spherical::toCartesian( void )
    {
      return sphericalToCartesian(_radius, _polar, _elevation);
    }
    Spherical Spherical::cartesianToSpherical( glm::vec3 cartCoords )
    {
      if ( cartCoords.x == 0.0f )
      {
        cartCoords.x = FLT_EPSILON;
      }
      Spherical res;
      res._radius = ( float) sqrt( ( cartCoords.x * cartCoords.x )
                  + ( cartCoords.y * cartCoords.y )
                  + ( cartCoords.z * cartCoords.z ) );
      res._polar = ( float ) atan(cartCoords.z / cartCoords.x );
      if ( cartCoords.x < 0.0f )
      {
        res._polar += Mathf::PI;
      }
      res._elevation = ( float ) std::asin( cartCoords.y / res._radius );
      return res;
    }
    glm::vec3 Spherical::sphericalToCartesian( float r, float p, float e )
    {
      float a = ( float ) ( r * cos( e ) );
      return glm::vec3(
        ( float ) ( a * cos( p ) ),
        ( float ) ( r * sin( e ) ),
        ( float ) ( a * sin( p ) )
      );
    }
  }
}
