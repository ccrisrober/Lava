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

#ifndef __POMPEII_ENGINE_COMPUTEBATCHQUEUE__
#define __POMPEII_ENGINE_COMPUTEBATCHQUEUE__

#include "Visitor.h"
#include <pompeiiEngine/api.h>
#include <pompeiiEngine/Scenegraph/Camera.h>
#include <pompeiiEngine/Rendering/BatchQueue.h>

#include <memory>

namespace pompeii
{
  namespace engine
  {
    class ComputeBatchQueue
      : public Visitor
    {
    public:
      POMPEIIENGINE_API
      ComputeBatchQueue( Camera* camera, std::shared_ptr<BatchQueue> bq );
      POMPEIIENGINE_API
      virtual void traverse( Node* n ) override;
      POMPEIIENGINE_API
      virtual void visitGroup( Group* g ) override;
      POMPEIIENGINE_API
      virtual void visitGeometry( Geometry* g ) override;
      POMPEIIENGINE_API
      virtual void visitLight( Light* l ) override;
    protected:
      Camera* _camera;
      std::shared_ptr<BatchQueue> _batch;
    };
  }
}

#endif /* __POMPEII_ENGINE_COMPUTEBATCHQUEUE__ */