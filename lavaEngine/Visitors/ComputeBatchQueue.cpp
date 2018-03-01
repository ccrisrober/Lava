#include "ComputeBatchQueue.h"

#include <lavaEngine/Scenegraph/Group.h>
#include <lavaEngine/Scenegraph/Light.h>
#include <lavaEngine/Scenegraph/Geometry.h>

namespace lava
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