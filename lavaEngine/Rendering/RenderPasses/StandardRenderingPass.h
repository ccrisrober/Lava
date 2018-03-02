#ifndef __LAVAENGINE_STANDARDRENDERINGPASS__
#define __LAVAENGINE_STANDARDRENDERINGPASS__

#include "RenderingPass.h"

namespace lava
{
  namespace engine
  {
    class StandardRenderingPass : public RenderingPass
    {
    public:
      LAVAENGINE_API
      void beginRender( Renderer* renderer, std::shared_ptr<BatchQueue>& bq );
      LAVAENGINE_API
      void render( Renderer* renderer, std::shared_ptr<BatchQueue>& bq, 
        Camera* c );

    protected:
      void beginRenderOpaqueObjects( Renderer* renderer, 
        std::shared_ptr<BatchQueue>& bq );
      void beginRenderTransparentObjects( Renderer* renderer, 
        std::shared_ptr<BatchQueue>& bq );
      void beginRenderStandard( Renderer* r, Renderable& renderable );

      void renderOpaqueObjects( Renderer* renderer, 
        std::shared_ptr<BatchQueue>& bq, Camera* c );
      void renderTransparentObjects( Renderer* renderer, 
        std::shared_ptr<BatchQueue>& bq, Camera* c );
      void renderStandardGeometry( Renderer*, Renderable&/*, MaterialPtr*/ );
    };
  }
}

#endif /* __LAVAENGINE_STANDARDRENDERINGPASS__ */