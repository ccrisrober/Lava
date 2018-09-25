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

#include "StandardRenderingPass.h"

namespace pompeii
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
      //const Color& cc = c->getClearColor( );
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

    void StandardRenderingPass::beginRenderStandard( Renderer*, 
      Renderable& renderable )
    {
      renderable.geometry->forEachPrimitive( [ & ]( std::shared_ptr<Primitive> )
      {
        std::cout << " TODO: Drawing primitive" << std::endl;
      } );
    }

    void StandardRenderingPass::renderOpaqueObjects( Renderer* renderer, 
      std::shared_ptr<BatchQueue>& bq, Camera* )
    {
      auto renderables = bq->renderables( BatchQueue::RenderableType::OPAQUE );
      if ( renderables.empty( ) )
      {
        return;
      }
      //    std::cout << "Render OpaqueObjects" << std::endl;
      //glm::mat4 projection = bq->getProjectionMatrix( );
      //glm::mat4 view = bq->getViewMatrix( );

      //uint32_t numLights = bq->_lights.size( );
      for ( auto& renderable : renderables )
      {
        // TODO: Set all uniforms!

        renderStandardGeometry( renderer, renderable/*, material*/ );
      }
    }

    void StandardRenderingPass::renderTransparentObjects( Renderer* renderer, 
      std::shared_ptr<BatchQueue>& bq, Camera* )
    {
      auto renderables = bq->renderables( BatchQueue::RenderableType::TRANSPARENT );
      if ( renderables.empty( ) )
      {
        return;
      }
      //std::cout << "Render TransparentObjects" << std::endl;
      //glm::mat4 projection = bq->getProjectionMatrix( );
      //glm::mat4 view = bq->getViewMatrix( );
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
        [ & ]( std::shared_ptr<Primitive> )
      {
        std::cout << "TODO: Drawing primitive" << std::endl;
      } );
    }
  }
}