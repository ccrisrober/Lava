#ifndef __LAVAENGINE_SPHERICAL__
#define __LAVAENGINE_SPHERICAL__

#include <lavaEngine/api.h>

#include <glm/glm.hpp>

namespace lava
{
  namespace engine
  {
    class Spherical
    {
    public:
      LAVAENGINE_API
      glm::vec3 toCartesian( void );
      LAVAENGINE_API
      static Spherical cartesianToSpherical( glm::vec3 cartCoords );
      LAVAENGINE_API
      static glm::vec3 sphericalToCartesian( float r, float p, float e );
    protected:
      float _radius;
      float _polar;
      float _elevation;
    };
  }
}

#endif /* __LAVAENGINE_SPHERICAL__ */
