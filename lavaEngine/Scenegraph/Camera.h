#pragma once

#include "Node.h"
#include <lavaEngine/Mathematics/Frustum.h>
#include <lavaEngine/Mathematics/Ray.h>

namespace lava
{
	namespace engine
	{
    typedef glm::vec4 Viewport;
		class Camera: public Node
		{
    public:
      LAVAENGINE_API
      static Camera* getMainCamera( void )
      {
        return _mainCamera;
      }
      LAVAENGINE_API
      static void setMainCamera( Camera* camera )
      {
        _mainCamera = camera;
      }
    private:
      // TODO shared_ptr??
      static Camera* _mainCamera;
    public:
      LAVAENGINE_API
      explicit Camera( void );
      LAVAENGINE_API
      Camera( const float fov, const float ar, const float n, const float f );
      LAVAENGINE_API
      virtual ~Camera( void );
    public:
      LAVAENGINE_API
      virtual void accept( Visitor& v ) override;
    public:
      LAVAENGINE_API
      bool isMainCamera( void ) const
      {
        return _isMainCamera;
      }
      LAVAENGINE_API
      void setIsMainCamera( bool v )
      {
        _isMainCamera = v;
      }
    protected:
      bool _isMainCamera = false;
    public:
      LAVAENGINE_API
      const glm::mat4& getProjection( void ) const;
      LAVAENGINE_API
      void setProjection( const glm::mat4& proj );
      LAVAENGINE_API
      const glm::mat4& getOrtographic( void );
      LAVAENGINE_API
      void setOrtographic( const glm::mat4 ortho );
      LAVAENGINE_API
      const glm::mat4& getView( void );
      LAVAENGINE_API
      void setView( const glm::mat4 view );
    protected:
      glm::mat4 _projectionMatrix;
      glm::mat4 _orthographicMatrix;
      glm::mat4 _viewMatrix;
    public:
      LAVAENGINE_API
      const Frustum& getFrustum( void ) const
      {
        return _frustum;
      }
      LAVAENGINE_API
      void setFrustum( const Frustum& frustum );
    protected:
      Frustum _frustum;

    public:
      LAVAENGINE_API
      const glm::vec4& getClearColor( void ) const
      {
        return _clearColor;
      }
      LAVAENGINE_API
      void setClearColor( const glm::vec4& color )
      {
        _clearColor = color;
      }
    protected:
      glm::vec4 _clearColor;  // TODO: Use color
    public:
      LAVAENGINE_API
      const Viewport& getViewport( void ) const
      {
        return _viewport;
      }
      LAVAENGINE_API
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