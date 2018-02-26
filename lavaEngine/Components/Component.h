#ifndef __MB_COMPONENT__
#define __MB_COMPONENT__

#include <iostream>
#include <mb/api.h>

#include "../Maths/Clock.hpp"

namespace mb
{
  class Renderer;
  class Camera;
}

namespace mb
{
  typedef std::string ComponentUID;
  #define IMPLEMENT_COMPONENT(__CLASS__) \
  public: \
    static mb::ComponentUID StaticGetUID( ) { \
    static std::string sUID = #__CLASS__; \
    return ( mb::ComponentUID ) sUID; /* This will be unique! */ \
    } \
    virtual mb::ComponentUID GetUID( ) const { return StaticGetUID( ); }

  class Node;

  class Component
  {
    friend class Node;
  public:
    LAVAENGINE_API
    virtual ComponentUID GetUID( ) const = 0;
    LAVAENGINE_API
    virtual ~Component( void );
    LAVAENGINE_API
    Node* node( void );
    LAVAENGINE_API
    const Node* getNode( void ) const
    {
      return _node;
    }
    LAVAENGINE_API
    virtual void update( const mb::Clock& clock );
    LAVAENGINE_API
    virtual void start( void );
    LAVAENGINE_API
    // Invoked once when component is attached to a node
    virtual void onAttach( void );
    LAVAENGINE_API
    // Invoked once when component is detached from a node
    virtual void onDetach( void );
    LAVAENGINE_API
    bool isEnabled( void ) const;
    LAVAENGINE_API
    void enable( void );
    LAVAENGINE_API
    void disable( void );
    LAVAENGINE_API
    void setEnabled( const bool v );
    LAVAENGINE_API
    void toggle( void );
    LAVAENGINE_API
    virtual void onEnable( void ) { }
    LAVAENGINE_API
    virtual void onDisable( void ) { }

    // Unuse
    virtual void onRenderDebug( Renderer*, Camera* ) { }
  private:
    void setNode( Node* n );
  protected:
    bool _enabled;
    LAVAENGINE_API
    Component( );
    Node* _node;
  };
}

#endif /* __MB_COMPONENT__ */
