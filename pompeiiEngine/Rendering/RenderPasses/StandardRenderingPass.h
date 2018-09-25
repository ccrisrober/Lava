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

#ifndef __POMPEIIENGINE_STANDARDRENDERINGPASS__
#define __POMPEIIENGINE_STANDARDRENDERINGPASS__

#include "RenderingPass.h"

namespace pompeii
{
  namespace engine
  {
    class StandardRenderingPass : public RenderingPass
    {
    public:
      POMPEIIENGINE_API
      void beginRender( Renderer* renderer, std::shared_ptr<BatchQueue>& bq );
      POMPEIIENGINE_API
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

#endif /* __POMPEIIENGINE_STANDARDRENDERINGPASS__ */