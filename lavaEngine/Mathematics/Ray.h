#ifndef __LAVAENGINE_RAY__
#define __LAVAENGINE_RAY__

#include <lavaEngine/api.h>

#include <glm/glm.hpp>

namespace lava
{
  namespace engine
  {
    class Ray
    {
    public:
      LAVAENGINE_API
        Ray( const glm::vec3& origin = glm::vec3( 0.0f ),
          const glm::vec3& direction = glm::vec3( 0.0f ) );
      LAVAENGINE_API
        glm::vec3 origin( void ) const;
      LAVAENGINE_API
        glm::vec3 direction( void ) const;
      LAVAENGINE_API
        void origin( const glm::vec3& ori );
      LAVAENGINE_API
        void direction( const glm::vec3& dir );
      LAVAENGINE_API
        glm::vec3 getPoint( const float& t );

      Ray &operator=( const Ray& r )
      {
        _origin = r._origin;
        _direction = r._direction;
        return *this;
      }
      bool operator==( const Ray& r )
      {
        return ( _origin == r._origin && _direction == r._direction );
      }

      bool operator!=( const Ray& r )
      {
        return !( *this == r );
      }

      //bool intersect( const BoundingVolume* );
    protected:
      glm::vec3 _origin;
      glm::vec3 _direction;
    };
  }
}

#endif /* __LAVAENGINE_RAY__ */
