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
    void Camera::setView( const glm::mat4 view )
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
      glm::vec3 position = getAbsolutePosition( );
      glm::vec3 dir; // TODO: = glm::normalize( glm::vec3( getAbsoluteRotation( ) ) );

      //_cullingPlanes[ 0 ] = Plane( dir, position + getFrustum( ).getDMin( ) * dir );
    }
  }
}