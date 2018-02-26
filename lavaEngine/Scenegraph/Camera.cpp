#include "Camera.h"
#include <iostream>

namespace lava
{
  namespace engine
  {
    Camera* Camera::_mainCamera = nullptr;

    Camera::Camera( void )
      : Camera( 45.0f, 1.0f, 0.1f, 1000.0f )
    {
    }
    Camera::Camera( const float fov, const float ar, 
      const float n, const float f )
      : Node( std::string( "Camera" ) )
    {
      _projectionMatrix = glm::mat4( 1.0f );
      _orthographicMatrix = glm::mat4( 1.0f );
      _viewMatrix = glm::mat4( 1.0f );
    }
    Camera::~Camera( void )
    {
      std::cout << "[D] Camera '" << this->name( ) << "'" << std::endl;
      if ( Camera::getMainCamera( ) == this )
      {
        setMainCamera( nullptr );
      }
    }
    void Camera::accept( Visitor& v )
    {
      v.visitCamera( this );
    }
    const glm::mat4 & Camera::getProjection( void ) const
    {
      return _projectionMatrix;
    }
    void Camera::setProjection( const glm::mat4 & proj )
    {
      _projectionMatrix = proj;
    }
    const glm::mat4 & Camera::getOrtographic( void )
    {
      return _orthographicMatrix;
    }
    void Camera::setOrtographic( const glm::mat4 ortho )
    {
      _orthographicMatrix = ortho;
    }
    const glm::mat4& Camera::getView( void )
    {
      return glm::mat4( 1.0f );
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
  }
}