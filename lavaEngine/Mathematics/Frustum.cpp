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

#include "Frustum.h"
#include <glm/gtc/type_ptr.hpp>

namespace lava
{
  namespace engine
  {
    Frustum::Frustum( void )
    {
    }
    Frustum::Frustum( const float& fov, const float& ar,
      const float& near, const float& far )
    {
      _data[ FRUSTUM_UMAX ] = near * std::tan( 0.5f * fov * 3.1415f / 180.0f );
      _data[ FRUSTUM_RMAX ] = ar * _data[ FRUSTUM_UMAX ];
      _data[ FRUSTUM_UMIN ] = -_data[ FRUSTUM_UMAX ];
      _data[ FRUSTUM_RMIN ] = -_data[ FRUSTUM_RMAX ];
      _data[ FRUSTUM_DMIN ] = near;
      _data[ FRUSTUM_DMAX ] = far;
    }

    glm::mat4 Frustum::computeProjMatrix( void ) const
    {
      float left = getRMin( );
      float right = getRMax( );
      float top = getUMax( );
      float bottom = getUMin( );
      float far = getDMax( );
      float near = getDMin( );

      std::array<float, 16> matrix;
      matrix[ 0 ] = 2.0f * near / ( right - left );
      matrix[ 1 ] = 0.0f;
      matrix[ 2 ] = 0.0f;
      matrix[ 3 ] = 0.0f;

      matrix[ 4 ] = 0.0f;
      matrix[ 5 ] = 2.0f * near / ( top - bottom );
      matrix[ 6 ] = 0.0f;
      matrix[ 7 ] = 0.0f;

      matrix[ 8 ] = ( right + left ) / ( right - left );
      matrix[ 9 ] = ( top + bottom ) / ( top - bottom );
      matrix[ 10 ] = -( far + near ) / ( far - near );
      matrix[ 11 ] = -1.0f;

      matrix[ 12 ] = 0.0f;
      matrix[ 13 ] = 0.0f;
      matrix[ 14 ] = -( 2.0f * far * near ) / ( far - near );
      matrix[ 15 ] = 0.0f;

      return glm::make_mat4( matrix.data( ) );
    }
    glm::mat4 Frustum::computeOthoMatrix( void ) const
    {
      float near = getDMin( );
      float far = getDMax( );
      float fov = getRMax( ) / getUMax( );
      float right = fov;
      float left = -fov;
      float top = 1.0f;
      float bottom = -1.0f;

      std::array<float, 16> matrix;
      matrix[ 0 ] = ( 2.0f / ( right - left ) );
      matrix[ 1 ] = 0.0f;
      matrix[ 2 ] = 0.0f;
      matrix[ 3 ] = -( right + left ) / ( right - left );

      matrix[ 4 ] = 0.0f;
      matrix[ 5 ] = ( 2.0f / ( top - bottom ) );
      matrix[ 6 ] = -( top + bottom ) / ( top - bottom );
      matrix[ 7 ] = 0.0f;

      matrix[ 8 ] = 0.0f;
      matrix[ 9 ] = 0.0f;
      matrix[ 10 ] = ( -2.0f / ( far - near ) );
      matrix[ 11 ] = ( far + near ) / ( far - near );

      matrix[ 12 ] = 0.0f;
      matrix[ 13 ] = 0.0f;
      matrix[ 14 ] = 0.0f;
      matrix[ 15 ] = 1.0f;

      return glm::make_mat4( matrix.data( ) );
    }
  }
}