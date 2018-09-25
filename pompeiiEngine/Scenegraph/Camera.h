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

#ifndef __POMPEII_ENGINE_CAMERA__
#define __POMPEII_ENGINE_CAMERA__

#include "Node.h"
#include <pompeiiEngine/Mathematics/Frustum.h>
#include <pompeiiEngine/Mathematics/Ray.h>

namespace pompeii
{
	namespace engine
	{
    typedef glm::vec4 Viewport;
		class Camera: public Node
		{
    public:
      static bool findCameras;
    public:
      POMPEIIENGINE_API
      static Camera* getMainCamera( void )
      {
        return _mainCamera;
      }
      POMPEIIENGINE_API
      static void setMainCamera( Camera* camera )
      {
        _mainCamera = camera;
      }
    private:
      // TODO shared_ptr??
      static Camera* _mainCamera;
    public:
      POMPEIIENGINE_API
      explicit Camera( void );
      POMPEIIENGINE_API
      Camera( const float fov, const float ar, const float n, const float f );
      POMPEIIENGINE_API
      virtual ~Camera( void );
    public:
      POMPEIIENGINE_API
      virtual void setEnabled( bool enabled ) 
      {
        Node::setEnabled( enabled );
        Camera::findCameras = true;
      }
      POMPEIIENGINE_API
      virtual void accept( Visitor& v ) override;
    public:
      POMPEIIENGINE_API
      bool isMainCamera( void ) const
      {
        return _isMainCamera;
      }
      POMPEIIENGINE_API
      void setIsMainCamera( bool v )
      {
        _isMainCamera = v;
      }
    protected:
      bool _isMainCamera = false;
    public:
      POMPEIIENGINE_API
      const glm::mat4& getProjection( void ) const;
      POMPEIIENGINE_API
      void setProjection( const glm::mat4& proj );
      POMPEIIENGINE_API
      const glm::mat4& getOrtographic( void );
      POMPEIIENGINE_API
      void setOrtographic( const glm::mat4 ortho );
      POMPEIIENGINE_API
      const glm::mat4& getView( void );
      POMPEIIENGINE_API
      void setView( const glm::mat4 view );
    protected:
      glm::mat4 _projectionMatrix;
      glm::mat4 _orthographicMatrix;
      glm::mat4 _viewMatrix;
    public:
      POMPEIIENGINE_API
      const Frustum& getFrustum( void ) const
      {
        return _frustum;
      }
      POMPEIIENGINE_API
      void setFrustum( const Frustum& frustum );
    protected:
      Frustum _frustum;

    public:
      POMPEIIENGINE_API
      const glm::vec4& getClearColor( void ) const
      {
        return _clearColor;
      }
      POMPEIIENGINE_API
      void setClearColor( const glm::vec4& color )
      {
        _clearColor = color;
      }
    protected:
      glm::vec4 _clearColor;  // TODO: Use color
    public:
      POMPEIIENGINE_API
      const Viewport& getViewport( void ) const
      {
        return _viewport;
      }
      POMPEIIENGINE_API
      void setViewport( const Viewport& v )
      {
        this->_viewport = v;
      }
    protected:
      Viewport _viewport;
    public:
      Ray getRay( float px, float py ) /* TODO: const*/
      {
        float x = 2.0f * px - 1.0f;
        float y = 1.0 - 2.0f * py;

        glm::vec4 rayClip( x, y, -1.0f, 1.0f );

        glm::vec4 rayEye = glm::transpose( glm::inverse( getProjection( ) ) ) 
          * rayClip;
        rayEye = glm::vec4( rayEye.x, rayEye.y, -1.0f, 0.0f );
        
        glm::vec3 rayDir = glm::normalize( 
          glm::vec3( glm::transpose( getTransform( ) ) * rayEye ) );

        return Ray( getAbsolutePosition( ), rayDir );
      }
      void computeCullingPlanes( void );
    private:
      bool _cullingEnabled = true;
      /*struct Plane
      {
        glm::vec3 normal;
        float distance;
        bool forceNormalize;
        Plane( glm::vec3 n, float d, bool f = true )
        {
          normal = n;
          distance = d;
          forceNormalize = f;
        }
      };
      std::array< Plane, 6 > _cullingPlanes;*/
		};
	}
}

#endif /* __POMPEII_ENGINE_CAMERA__ */