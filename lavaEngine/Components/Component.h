#ifndef __LAVAENGINE_COMPONENT__
#define __LAVAENGINE_COMPONENT__

#include <iostream>
#include <lavaEngine/api.h>

//#include "../Maths/Clock.hpp"

namespace lava
{
  namespace engine
  {
    //class Renderer;
    class Camera;
  }
}

namespace lava
{
  namespace engine
  {
    typedef std::string ComponentUID;
    #define IMPLEMENT_COMPONENT(__CLASS__) \
    public: \
      static lava::engine::ComponentUID StaticGetUID( void ) { \
      static std::string sUID = #__CLASS__; \
      return ( lava::engine::ComponentUID ) sUID; /* This will be unique! */ \
      } \
      virtual lava::engine::ComponentUID GetUID( void ) const{ return StaticGetUID( ); }

    class Node;

    class Component
    {
      friend class Node;
    public:
      LAVAENGINE_API
      virtual ComponentUID GetUID( void ) const = 0;
      LAVAENGINE_API
      virtual ~Component( void );
      LAVAENGINE_API
      Node* node( void );
      LAVAENGINE_API
      const Node* getNode( void ) const
      {
        return _node;
      }
      // TODO: LAVAENGINE_API
      // TODO: virtual void update( const mb::Clock& clock );
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
      // TODO: virtual void onRenderDebug( Renderer*, Camera* ) { }
    private:
      void setNode( Node* n );
    protected:
      bool _enabled;
      LAVAENGINE_API
      Component( void );
      Node* _node;
    };
  }
}

#endif /* __LAVAENGINE_COMPONENT__ */
