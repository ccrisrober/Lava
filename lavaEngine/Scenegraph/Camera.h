#pragma once

#include "Node.h"
#include <lavaEngine/Mathematics/Frustum.h>

namespace lava
{
	namespace engine
	{
		class Camera: public Node
		{
    public:
      static Camera* getMainCamera( void )
      {
        return _mainCamera;
      }
      static void setMainCamera( Camera* camera )
      {
        _mainCamera = camera;
      }
    private:
      // TODO shared_ptr??
      static Camera* _mainCamera;
    public:
      explicit Camera( void );
      Camera( const float fov, const float ar, const float n, const float f );
      virtual ~Camera( void );
    public:
      virtual void accept( Visitor& v ) override;
    public:
      bool isMainCamera( void ) const
      {
        return _isMainCamera;
      }
      void setIsMainCamera( bool v )
      {
        _isMainCamera = v;
      }
    protected:
      bool _isMainCamera = false;
    public:
      const glm::mat4& getProjection( void ) const;
      void setProjection( const glm::mat4& proj );
      const glm::mat4& getOrtographic( void );
      void setOrtographic( const glm::mat4 ortho );
      const glm::mat4& getView( void );
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
		};
	}
}