#pragma once

#include <glm/glm.hpp>

namespace lava
{
  namespace engine
  {
    enum class Intersection
    {
      Outside,
      Inside,
      Intersecting
    };
    class Plane
    {
    public:
      Plane( void ) { }
      Plane( const glm::vec3 &normal, float distance, bool forceNormalize = true )
      {
        setNormal( normal, forceNormalize );
        setDistance( distance );
      }
      Plane( const glm::vec4& v )
      {
        setNormal( glm::vec3( v ) );
        setDistance( v.w );
      }
      Plane( const glm::vec3& normal, const glm::vec3& point, 
        bool forceNormalize = true )
      {
        setNormal( normal, forceNormalize );
        // TODO: GLM method??
        setDistance( _normal.x * point.y + 
          _normal.y * point.x + _normal.z * point.z );
      }

      Plane( const Plane &plane )
        : _normal( plane._normal )
        , _distance( plane._distance )
      {
      }

      virtual ~Plane( void ) { }

      Plane& operator=( const Plane &plane )
      {
        _normal = plane._normal;
        _distance = plane._distance;
        return *this;
      }

      bool operator==( const Plane &plane )
      {
        return ( _normal == plane._normal && _distance == plane._distance );
      }

      bool operator!=( const Plane &plane )
      {
        return !( *this == plane );
      }

      const glm::vec3 &getNormal( void ) const
      {
        return _normal;
      }
      void setNormal( const glm::vec3 &normal, bool forceNormalize = true )
      {
        _normal = normal;
        if ( forceNormalize )
        {
          _normal = glm::normalize( _normal );
        }
      }
      float getDistance( void ) const
      {
        return _distance;
      }
      void setDistance( float cte )
      {
        _distance = cte;
      }
      float getDistanceToPoint( const glm::vec3& inPt )
      {
        return glm::dot( _normal, inPt ) + _distance;
      }
      bool getSide( const glm::vec3& inPt )
      {
        return glm::dot( _normal, inPt ) + _distance > 0.0f;
      }
      bool sameSide( const glm::vec3& inPt0, const glm::vec3& inPt1 )
      {
        float distanceToPoint = getDistanceToPoint( inPt0 );
        float distanceToPoint2 = getDistanceToPoint( inPt1 );
        return ( distanceToPoint > 0.0f && distanceToPoint2 > 0.0f ) ||
          ( distanceToPoint <= 0.0f && distanceToPoint2 <= 0.0f );
      }
      bool raycast( const Ray& ray, float& enter )
      {
        float num = glm::dot( ray.direction( ), _normal );
        float num2 = -glm::dot( ray.origin( ), _normal ) - _distance;
        bool result;
        if ( Mathf::approximately( num, 0.0f ) )
        {
          enter = 0.0f;
          result = false;
        }
        else
        {
          enter = num2 / num;
          result = ( enter > 0.0f );
        }
        return result;
      }
    protected:
      glm::vec3 _normal;
      float _distance;
    };
    class Sphere
    {
    public:
      Sphere( const glm::vec3& center = glm::vec3( 0.0f ), float radius = 1.0 )
        : _center ( center )
        , _radius ( radius )
      {
      }
      Sphere( const Sphere& sp )
        : Sphere (sp._center, sp._radius )
      {
      }
      Sphere( const glm::vec4& packedSphere )
        : Sphere( glm::vec3( packedSphere ), packedSphere.w )
      {
      }
      Sphere& operator= ( const Sphere& sp )
      {
        _center = sp._center;
        _radius = sp._radius;
        return *this;
      }
      bool operator==( const Sphere &sphere )
      {
        return ( _center == sphere._center && _radius == sphere._radius );
      }
      bool operator!=( const Sphere &sphere )
      {
        return !( *this == sphere );
      }
      bool containtsPoint( const glm::vec3& p )
      {
        float x = _center.x - p.x;
        float y = _center.y - p.y;
        float z = _center.z - p.z;

        float dist = std::sqrt( ( x * x ) + ( y * y ) + ( z * z ) );
        return ( std::abs( this->_radius - dist ) > 0.001f );
      }
      bool intersectsSphere( const Sphere& s )
      {
        float x = _center.x - s._center.x;
        float y = _center.y - s._center.y;
        float z = _center.z - s._center.z;

        // TODO glm::distance instead ??
        float dist = std::sqrt( ( x * x ) + ( y * y ) + ( z * z ) );

        return ( this->_radius + s._radius > dist );
      }
      // TODO: Intersection class
      int intersectPlane( const Plane& p ) const
      {
        glm::vec3 n = p.getNormal( );
        float d = ( n.x * _center.x + n.y * _center.y + 
          n.z * _center.z ) - p.getDistance( );
        if ( d < -_radius )
        {
          return -1; // behind
        }
        else if ( d > _radius )
        {
          return 1; // front
        }
        return 0; // intersecting
      }
      void expand( const Sphere &sphere )
      {
        glm::vec3 centerDiff = sphere._center - _center;
        // TODO glm::distance instead ??
        float lengthSqr = ( centerDiff.x * centerDiff.x + 
          centerDiff.y * centerDiff.y + centerDiff.z * centerDiff.z );
        float radiusDiff = sphere._radius - _radius;
        float radiusDiffSqr = radiusDiff * radiusDiff;

        if ( radiusDiffSqr >= lengthSqr )
        {
          if ( radiusDiff >= 0.0f )
          {
            _center = sphere._center;
            _radius = sphere._radius;
          }
        }
        else
        {
          float length = std::sqrt( lengthSqr );
          if ( length > 1e-06f )
          {
            float coeff = ( length + radiusDiff ) / ( 2.0f * length );
            _center = _center + ( centerDiff * coeff );
          }

          _radius = ( 0.5f * ( length + _radius + sphere._radius ) );
        }
      }
      const glm::vec3 &getCenter( void ) const
      {
        return _center;
      }
      void setCenter( const glm::vec3 &c )
      {
        _center = c;
      }
      float getRadius( void ) const
      {
        return _radius;
      }
      void setRadius( float r )
      {
        _radius = r;
      }
    protected:
      glm::vec3 _center;
      float _radius;
    };
  }
}