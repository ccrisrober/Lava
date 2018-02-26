#include "Component.h"
#include <lavaEngine/Scenegraph/Node.h>
#include <string>

//#include <lava/lava.h>

namespace lava
{
  namespace engine
  {
    Component::Component( void )
      : _enabled( true )
      , _node( nullptr )
    {
    }

    Component::~Component( void )
    {
      // lava::Log::debug("[D] Component");
    }

    Node* Component::node( void )
    {
      return _node;
    }

    void Component::setNode( Node* n )
    {
      _node = n;
    }

    /* TODO: void Component::update( const mb::Clock& )
    {
    }*/

    void Component::start( void )
    {
      // lava::Log::debug("Init ", GetUID( ), " component");
    }

    void Component::onAttach( void )
    {
      // lava::Log::debug("Attached ", this->GetUID( ), " to node '",
      //  this->node( )->name( ), "'");
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
      if ( _enabled )
      {
        onEnable( );
      }
      else
      {
        onDisable( );
      }
    }
    void Component::toggle( void )
    {
      setEnabled( !isEnabled( ) );
    }
  }
}
