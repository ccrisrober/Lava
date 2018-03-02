#ifndef __LAVAENGINE_STATE_MACHINE__
#define __LAVAENGINE_STATE_MACHINE__

#include <lavaEngine/api.h>
#include <memory>
#include <string>

#include "Component.h"

namespace lava
{
  namespace engine
  {
    class StateMachine;
    typedef std::shared_ptr<StateMachine> StateMachinePtr;
    class State
    {
    public:
      State( StateMachinePtr owner_ );
      virtual ~State( void );
      virtual void enter( void ) = 0;
      virtual void exit( void ) = 0;
      virtual void update( void ) = 0;
      virtual std::string description( void ) = 0;
      StateMachinePtr owner;
    };
    typedef std::shared_ptr<State> StatePtr;
    class StateMachine: public Component
    {
      IMPLEMENT_COMPONENT( StateMachine )
    public:
      LAVAENGINE_API
      StateMachine( void );
      LAVAENGINE_API
      ~StateMachine( void );

      StatePtr currentState;

      LAVAENGINE_API
      virtual void update( const float& );
      LAVAENGINE_API
      void SwitchState( StatePtr newState );
    };
  }
}

#endif /* __LAVAENGINE_STATE_MACHINE__ */
