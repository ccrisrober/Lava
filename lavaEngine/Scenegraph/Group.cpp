#include "Group.h"
#include <iostream>
#include <algorithm>

namespace lava
{
  namespace engine
  {
    Group::Group( const std::string name )
      : Node( name )
    {
    }

    Group::~Group( void )
    {
      std::cout << "[D] Group '" << this->name( ) << "'" << std::endl;
      removeChildren( );
    }

    bool Group::hasNodes( void ) const
    {
      return !_children.empty( );
    }
    unsigned int Group::getNumChildren( void ) const
    {
      return _children.size( );
    }

    void Group::addChild( Node* node )
    {
      if ( node->parent( ) == this )
      {
        throw /*RuntimeException*/( "You trying to add this node repeated" );
      }
      if ( node->parent( ) != nullptr )
      {
        throw /*HasParentException*/( node->name( ),
          name( ), node->parent( )->name( ) );
      }
      node->parent( this );
      _children.push_back( node );
    }

    void Group::removeChild( Node* n )
    {
      if ( n->parent( ) == this )
      {
        _children.erase( std::remove( _children.begin( ), _children.end( ), n ), 
          _children.end( ) );
        n->parent( nullptr );
      }
    }

    void Group::removeChildren( void )
    {
      for ( auto& child : _children )
      {
        child->parent( nullptr );
        delete child;
      }
      _children.clear( );
    }

    Node* Group::nodeAt( unsigned int idx )
    {
      return _children.at( idx );
    }


    void Group::insertChild( unsigned int idx, Node* node )
    {
      _children.insert( _children.begin( ) + idx, node );
    }

    void Group::removeChild( unsigned int idx )
    {
      if ( _children.size( ) > idx )
      {
        _children.erase( _children.begin( ) + idx );
      }
    }

    void Group::forEachNode( std::function< void( Node * ) > callback )
    {
      for ( auto& child : _children )
      {
        callback( child );
      }
    }
    void Group::accept( Visitor& v )
    {
      v.visitGroup( this );
    }
  }
}