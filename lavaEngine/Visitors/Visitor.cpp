#include "Visitor.h"

#include <lavaEngine/Scenegraph/Node.h>
#include <lavaEngine/Scenegraph/Group.h>
#include <lavaEngine/Scenegraph/Geometry.h>
#include <lavaEngine/Scenegraph/Camera.h>
#include <lavaEngine/Scenegraph/Light.h>

namespace lava
{
  namespace engine
  {
    Visitor::~Visitor( void )
    {
    }

    Visitor::Visitor( const Visitor& )
    {
    }

    Visitor &Visitor::operator= ( const Visitor& )
    {
      return *this;
    }

    void Visitor::reset( void )
    {
    }

    void Visitor::traverse( Node* n )
    {
      reset( );
      n->accept( *this );
    }

    void Visitor::visitNode( Node* )
    {
    }

    void Visitor::visitGroup( Group* group )
    {
      group->forEachNode( [ & ]( Node* n )
      {
        n->accept( *this );
      } );
    }

    void Visitor::visitLight( Light* light )
    {
      visitNode( light );
    }

    void Visitor::visitCamera( Camera *camera )
    {
      visitNode( camera );
    }

    void Visitor::visitGeometry( Geometry *geometry )
    {
      visitNode( geometry );
    }
  }
}
