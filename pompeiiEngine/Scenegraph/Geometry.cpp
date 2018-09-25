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

#include "Geometry.h"
#include <iostream>

namespace pompeii
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

    /*void Geometry::removePrimitive( std::shared_ptr<Primitive> p )
    {
      _primitives.erase( std::find( _primitives.begin( ), 
        _primitives.end( ), p ) );
    }*/

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