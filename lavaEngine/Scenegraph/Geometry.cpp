#include "Geometry.h"
#include <iostream>

namespace lava
{
  namespace engine
  {
    Geometry::Geometry( const std::string & name )
      : Node( name )
    {
      // TODO: Add mesh and material component??
    }

    Geometry::~Geometry( void )
    {
      std::cout << "[D] Geometry '" << this->name( ) << "'" << std::endl;
      removeAllPrimitives( );
    }

    void Geometry::addPrimitive( std::shared_ptr<Primitive> p )
    {
      _primitives.push_back( p );
    }

    bool Geometry::hasPrimitives( void ) const
    {
      return !_primitives.empty( );
    }

    void Geometry::removePrmitive( std::shared_ptr<Primitive> p )
    {
      _primitives.erase( std::find( _primitives.begin( ), 
        _primitives.end( ), p ) );
    }

    void Geometry::removeAllPrimitives( void )
    {
      _primitives.clear( );
    }

    void Geometry::forEachPrimitive( std::function<void( 
      std::shared_ptr<Primitive> )> callback )
    {
      for ( auto& p : _primitives )
      {
        callback( p );
      }
    }

    void Geometry::accept( Visitor& v )
    {
      v.visitGeometry( this );
    }

  }
}