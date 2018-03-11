#ifndef __LAVA_ENGINE_FRUSTUM__
#define __LAVA_ENGINE_FRUSTUM__

#include <array>
#include <iomanip>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    // Ideas from https://sites.google.com/site/letsmakeavoxelengine/home/frustum-culling
    class Frustum
    {
      enum {
        FRUSTUM_UMIN = 2,
        FRUSTUM_UMAX = 3,
        FRUSTUM_RMIN = 0,
        FRUSTUM_RMAX = 1,
        FRUSTUM_DMIN = 4,
        FRUSTUM_DMAX = 5,
      };
    public:
      LAVAENGINE_API
      Frustum( void );
      LAVAENGINE_API
      Frustum( const float& fov, const float& ar,
        const float& near, const float& far );

      Frustum( const Frustum& f )
      {
        std::copy( std::begin( f._data ),
          std::end( f._data ), std::begin( _data ) );
      }
      bool operator==( const Frustum &f ) const
      {
        return std::equal( _data.begin( ), _data.end( ), f._data.begin( ) );
      }

      bool operator!=( const Frustum& f ) const
      {
        return !( *this == f );
      }

      LAVAENGINE_API
      float getFOV( void ) const
      {
        return getUMax( ) / getDMin( );
      }
      LAVAENGINE_API
      float getAspect( void ) const
      {
        return getRMax( ) / getUMax( );
      }
      LAVAENGINE_API
      float getLinearDepth( void ) const
      {
        return getDMax( ) - getDMin( );
      }

      /*LAVAENGINE_API
      void setFieldOfView( float fov )
      {
      _data[ FRUSTUM_UMAX ] = near * std::tan( 0.5f * fov * 3.1415f / 180.0f );
      }*/

      float getRMin( void ) const { return _data[ FRUSTUM_RMIN ]; }
      float getRMax( void ) const { return _data[ FRUSTUM_RMAX ]; }
      float getUMin( void ) const { return _data[ FRUSTUM_UMIN ]; }
      float getUMax( void ) const { return _data[ FRUSTUM_UMAX ]; }
      float getDMin( void ) const { return _data[ FRUSTUM_DMIN ]; }
      float getDMax( void ) const { return _data[ FRUSTUM_DMAX ]; }

      glm::mat4 computeProjMatrix( void ) const;
      glm::mat4 computeOthoMatrix( void ) const;

      friend std::ostream& operator<<( std::ostream &out, const Frustum& f )
      {
        out << std::setiosflags( std::ios::fixed | std::ios::showpoint )
          << std::setprecision( 4 )
          << "[D = (" << f.getDMin( ) << ", " << f.getDMax( ) << "), "
          << "R = (" << f.getRMin( ) << ", " << f.getRMax( ) << "), "
          << "U = (" << f.getUMin( ) << ", " << f.getUMax( ) << ")]";
        return out;
      }
    protected:
      std::array< float, 6 > _data;
    };
  }
}

#endif /* __LAVA_ENGINE_FRUSTUM__ */