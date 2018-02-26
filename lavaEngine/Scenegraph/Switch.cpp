#include "Switch.h"
#include <iostream>

namespace lava
{
  namespace engine
  {
    Switch::Switch( const std::string& name )
      : Group( name )
      , _activeChild( SW_INVALID_CHILD )
    {
    }

    Switch::~Switch( void )
    {
      std::cout << "[D] Switch '" << this->name( ) << "'" << std::endl;
    }

    void Switch::forEachNode( std::function<void( Node* )> cb )
    {
      if ( !hasNodes( ) )
      {
        return;
      }
      auto current = nodeAt( _activeChild );
      if ( current != nullptr )
      {
        cb( current );
      }
    }

    void Switch::selectNextNode( void )
    {
      if ( !hasNodes( ) )
      {
        return;
      }
      _activeChild = ( _activeChild + 1 ) % getNumChildren( );
    }

    void Switch::selectPreviousNode( void )
    {
      if ( !hasNodes( ) )
      {
        return;
      }
      auto nChildren = getNumChildren( );
      _activeChild = ( _activeChild + nChildren - 1 ) % nChildren;
    }

    Node* Switch::currentNode( void )
    {
      return nodeAt( _activeChild );
    }

    unsigned int Switch::getActiveChild( void ) const
    {
      return _activeChild;
    }

    void Switch::setActiveChild( unsigned int idx )
    {
      _activeChild = idx;
    }
  }
}