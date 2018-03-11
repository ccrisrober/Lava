#ifndef __LAVA_ENGINE_BATCHQUEUE__
#define __LAVA_ENGINE_BATCHQUEUE__

#include <vector>
#include <map>

#include <lavaEngine/Scenegraph/Light.h>
#include <lavaEngine/Scenegraph/Geometry.h>
#include <lavaEngine/Scenegraph/Camera.h>

namespace lava
{
	namespace engine
	{
    struct Renderable
    {
      //MaterialPtr material;
      Geometry* geometry;
      glm::mat4 modelTransform;
      float zDistance;
      Renderable( /*MaterialPtr m,*/ Geometry* g,
        const glm::mat4& mt, float zDist )
      {
        //this->material = m;
        this->geometry = g;
        this->modelTransform = mt;
        this->zDistance = zDist;
      }
    };
    class BatchQueue
    {
    public:
      enum class RenderableType
      {
        OPAQUE,
        TRANSPARENT,
        SHADOW
      };
      LAVAENGINE_API
      BatchQueue( void );
      LAVAENGINE_API
      virtual ~BatchQueue( void );
      LAVAENGINE_API
      std::vector<Renderable> renderables( RenderableType t );
      LAVAENGINE_API
      void pushGeometry( Geometry* g );
      LAVAENGINE_API
      void pushLight( Light * l );
      LAVAENGINE_API
      void reset( void );
      LAVAENGINE_API
      void setCamera( Camera* c );
      LAVAENGINE_API
      Camera* getCamera( void );
      LAVAENGINE_API
      const glm::mat4& getProjectionMatrix( void ) const { return _projMatrix; }
      LAVAENGINE_API
      const glm::mat4& getViewMatrix( void ) const { return _viewMatrix; }
    protected:
      Camera* _camera;
    public:
      std::vector< Light* > _lights;
    protected:
      glm::mat4 _projMatrix;
      glm::mat4 _viewMatrix;
      std::map< RenderableType, std::vector< Renderable >> _renderables;
    };
	}
}

#endif /* __LAVA_ENGINE_BATCHQUEUE__ */