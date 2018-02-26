#include "Node.h"
#include <iostream>

namespace lava
{
	namespace engine
	{
		Node::Node( const std::string& name )
		{
		}
    Node::~Node( void )
    {
      std::cout << "[D] Node '" << this->name( ) << "'" << std::endl;
#ifdef LAVAENGINE_HASCOMPONENTS
      detachAllComponents( );
#endif
    }
    std::string Node::name( void ) const
    {
      return _name;
    }
    void Node::name( const std::string & name )
    {
      _name = name;
    }
    Node* Node::parent( void )
    {
      return _parent;
    }
    void Node::parent( Node * p )
    {
      _parent = p;
    }
    void Node::perform( Visitor& visitor )
    {
      visitor.traverse( this );
    }
    void Node::perform( const Visitor& visitor )
    {
      const_cast< Visitor& >( visitor ).traverse( this );
    }
    void Node::accept( Visitor& visitor )
    {
      visitor.visitNode( this );
    }
#ifdef LAVAENGINE_HASCOMPONENTS
    void Node::startComponents( void )
    {
      forEachComponent( [ ]( Component* c ) {
        c->start( );
      } );
    }
    void Node::addComponent( Component * comp )
    {
      comp->setNode( this );
      _components.insert( std::pair<std::string, Component*>( comp->GetUID( ), comp ) );
      comp->onAttach( );
    }
    void Node::detachAllComponents( void )
    {
      forEachComponent( [ ]( Component *cmp )
      {
        cmp->onDetach( );
        cmp->setNode( nullptr );
      } );

      _components.clear( );
    }
    void Node::forEachComponent( std::function<void( Component* )> callback )
    {
      // create a copy of the component's collection
      // to prevent errors when attaching or detaching
      // components during an update pass
      auto cs = _components;
      for ( auto cmp : cs )
      {
        if ( cmp.second != nullptr )
        {
          callback( cmp.second );
        }
      }
    }
#endif
	}
}