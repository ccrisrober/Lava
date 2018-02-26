#include "Component.h"
#include "../Scenegraph/Node.h"
#include <string>

#include "../Utils/Log.hpp"

namespace mb
{
  Component::Component( )
    : _enabled( true )
    , _node( nullptr )
  {
  }

  Component::~Component( )
  {
    mb::Log::debug("[D] Component");
  }

  Node* Component::node( )
  {
    return _node;
  }

  void Component::setNode( Node* n )
  {
    _node = n;
  }

  void Component::update( const mb::Clock& )
  {
  }

  void Component::start( void )
  {
    mb::Log::debug("Init ", GetUID( ), " component");
  }

  void Component::onAttach( void )
  {
    mb::Log::debug("Attached ", this->GetUID( ), " to node '",
      this->node( )->name( ), "'");
  }

  void Component::onDetach( void )
  {
  }
  bool Component::isEnabled( void ) const
  {
    return _enabled;
  }
  void Component::enable( void )
  {
    setEnabled( true );
  }
  void Component::disable( void )
  {
    setEnabled( false );
  }
  void Component::setEnabled( const bool v )
  {
    _enabled = v;
    if ( _enabled == true )
    {
      onEnable( );
    }
    else if ( _enabled == false )
    {
      onDisable( );
    }
  }
  void Component::toggle( void )
  {
    setEnabled( !isEnabled( ) );
  }
}
