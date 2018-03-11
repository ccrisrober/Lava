#ifndef __LAVA_ENGINE_FETCHCAMERAS__
#define __LAVA_ENGINE_FETCHCAMERAS__

#include <lavaEngine/Scenegraph/Camera.h>
#include <lavaEngine/api.h>
#include <vector>
#include <functional>

namespace lava
{
  namespace engine
  {
    class FetchCameras:
      public Visitor
    {
    public:
      LAVAENGINE_API
      virtual void reset( void ) override;
      LAVAENGINE_API
      virtual void visitCamera( Camera* c ) override;
      LAVAENGINE_API
      bool hasCameras( void ) const
      {
        return !_cameras.empty( );
      }
      LAVAENGINE_API
      void forEachCameras( std::function<void( Camera* )> cb );
    protected:
      std::vector< Camera* > _cameras;
      };
  }
}

#endif /* __LAVA_ENGINE_FETCHCAMERAS__ */