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

#include "ComputeBatchQueue.h"

#include <pompeiiEngine/Scenegraph/Group.h>
#include <pompeiiEngine/Scenegraph/Light.h>
#include <pompeiiEngine/Scenegraph/Geometry.h>

namespace pompeii
{
  namespace engine
  {
    ComputeBatchQueue::ComputeBatchQueue( Camera * camera, 
      std::shared_ptr<BatchQueue> bq )
      : _camera( camera )
      , _batch( bq )
    {
    }
    void ComputeBatchQueue::traverse( Node* node )
    {
      _batch->reset( );
      _batch->setCamera( _camera );

      /*if ( _camera != nullpr )
      {
      _camera->computeCullingPlanes( );
      }*/

      Visitor::traverse( node );
    }
    void ComputeBatchQueue::visitGroup( Group* group )
    {
      // No ejecutamos culling de la cámara
      //  sobre los grupos porque los hijos pueden
      //  tener nodos útiles (?)
      Visitor::visitGroup( group );
    }
    void ComputeBatchQueue::visitGeometry( Geometry* geom )
    {
      // TODO: Culled frustum culling

      if ( _camera != nullptr &&
        _camera->layer( ).check( geom->layer( ) ) )
      {
        _batch->pushGeometry( geom );
      }
    }

    void ComputeBatchQueue::visitLight( Light* light )
    {
      _batch->pushLight( light );
    }
  }
}