#include "StandardRenderingPass.h"

namespace lava
{
  namespace engine
  {
    void StandardRenderingPass::beginRender( Renderer* renderer, 
      std::shared_ptr<BatchQueue>& bq )
    {
      beginRenderOpaqueObjects( renderer, bq );
      beginRenderTransparentObjects( renderer, bq );
    }

    void StandardRenderingPass::render( Renderer* renderer, 
      std::shared_ptr<BatchQueue>& bq, Camera * c )
    {
      // Clear window (HARDCODED) TODO
      const Color& cc = c->getClearColor( );
      // TODO: glClearColor( cc.r( ), cc.g( ), cc.b( ), cc.a( ) );

      // computeShadows( )
      renderOpaqueObjects( renderer, bq, c );
      renderTransparentObjects( renderer, bq, c );
    }

    void StandardRenderingPass::beginRenderOpaqueObjects( Renderer* renderer, 
      std::shared_ptr<BatchQueue>& bq )
    {
      auto renderables = bq->renderables( BatchQueue::RenderableType::OPAQUE );
      if ( renderables.empty( ) )
      {
        return;
      }

      for ( Renderable& renderable : renderables )
      {
        beginRenderStandard( renderer, renderable );
      }
    }
    
    void StandardRenderingPass::beginRenderTransparentObjects( Renderer* renderer, 
      std::shared_ptr<BatchQueue>& bq )
    {
      auto renderables = bq->renderables( BatchQueue::RenderableType::TRANSPARENT );
      if ( renderables.empty( ) )
      {
        return;
      }

      for ( Renderable& renderable : renderables )
      {
        beginRenderStandard( renderer, renderable );
      }
    }

    void StandardRenderingPass::beginRenderStandard( Renderer* renderer, 
      Renderable& renderable )
    {
      renderable.geometry->forEachPrimitive( [ & ]( std::shared_ptr<Primitive> p )
      {
        std::cout << " TODO: Drawing primitive" << std::endl;
      } );
    }

    void StandardRenderingPass::renderOpaqueObjects( Renderer* renderer, 
      std::shared_ptr<BatchQueue>& bq, Camera * c )
    {
      auto renderables = bq->renderables( BatchQueue::RenderableType::OPAQUE );
      if ( renderables.empty( ) )
      {
        return;
      }
      //    std::cout << "Render OpaqueObjects" << std::endl;
      glm::mat4 projection = bq->getProjectionMatrix( );
      glm::mat4 view = bq->getViewMatrix( );

      uint32_t numLights = bq->_lights.size( );
      for ( auto& renderable : renderables )
      {
        // TODO: Set all uniforms!

        renderStandardGeometry( renderer, renderable/*, material*/ );
      }
    }

    void StandardRenderingPass::renderTransparentObjects( Renderer* renderer, 
      std::shared_ptr<BatchQueue>& bq, Camera * c )
    {
      auto renderables = bq->renderables( BatchQueue::RenderableType::TRANSPARENT );
      if ( renderables.empty( ) )
      {
        return;
      }
      //std::cout << "Render TransparentObjects" << std::endl;
      glm::mat4 projection = bq->getProjectionMatrix( );
      glm::mat4 view = bq->getViewMatrix( );
      for ( auto& renderable : renderables )
      {
        /*auto material = renderable.material;
        if ( !material ) continue;

        material->uniform( MB_PROJ_MATRIX )->value( projection );
        material->uniform( MB_VIEW_MATRIX )->value( view );*/

        renderStandardGeometry( renderer, renderable/*, material*/ );
      }
    }

    void StandardRenderingPass::renderStandardGeometry( Renderer*, 
      Renderable& renderable /*, MaterialPtr*/ )
    {
      renderable.geometry->forEachPrimitive( 
        [ & ]( std::shared_ptr<Primitive> p )
      {
        std::cout << "TODO: Drawing primitive" << std::endl;
      } );
    }
  }
}