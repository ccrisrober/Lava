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

#include "Camera.h"
#include <iostream>

namespace lava
{
  namespace engine
  {
    Camera* Camera::_mainCamera = nullptr;
    bool Camera::findCameras = true;

    Camera::Camera( void )
      : Camera( 45.0f, 1.0f, 0.1f, 1000.0f )
    {
    }
    Camera::Camera( const float fov, const float ar, 
      const float near, const float far )
      : Node( std::string( "Camera" ) )
      , _frustum( fov, ar, near, far )
      , _clearColor( glm::vec4( 0.2f, 0.3f, 0.3f, 1.0f ) )
      , _viewport( glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f ) )
    {
      for ( unsigned int i = 0; i < 32; ++i )
      {
        this->layer( ).enable( i );
      }
      _projectionMatrix = _frustum.computeProjMatrix( );
      _orthographicMatrix = _frustum.computeOthoMatrix( );
      _viewMatrix = glm::mat4( 1.0f );
    }
    Camera::~Camera( void )
    {
      std::cout << "[D] Camera '" << this->name( ) << "'" << std::endl;
      if ( Camera::getMainCamera( ) == this )
      {
        setMainCamera( nullptr );
      }
      Camera::findCameras = true;
    }
    void Camera::accept( Visitor& v )
    {
      v.visitCamera( this );
    }
    const glm::mat4& Camera::getProjection( void ) const
    {
      return _projectionMatrix;
    }
    void Camera::setProjection( const glm::mat4 & proj )
    {
      _projectionMatrix = proj;
    }
    const glm::mat4& Camera::getOrtographic( void )
    {
      return _orthographicMatrix;
    }
    void Camera::setOrtographic( const glm::mat4 ortho )
    {
      _orthographicMatrix = ortho;
    }
    const glm::mat4& Camera::getView( void )
    {
      _viewMatrix = glm::inverse( getTransform( ) );
      return _viewMatrix;
    }
    void Camera::setView( const glm::mat4 )
    {
      // TODO
    }
    void Camera::setFrustum( const Frustum& frustum )
    {
      _frustum = frustum;
      _projectionMatrix = _frustum.computeProjMatrix( );
      _orthographicMatrix = _frustum.computeOthoMatrix( );
    }
    void Camera::computeCullingPlanes( void )
    {
      //glm::vec3 position = getAbsolutePosition( );
      //glm::vec3 dir; // TODO: = glm::normalize( glm::vec3( getAbsoluteRotation( ) ) );

      //_cullingPlanes[ 0 ] = Plane( dir, position + getFrustum( ).getDMin( ) * dir );
    }
  }
}