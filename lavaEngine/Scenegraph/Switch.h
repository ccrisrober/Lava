#pragma once

#include "Group.h"
#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class Switch : public Group
    {
    public:
      LAVAENGINE_API
      Switch( const std::string& name );
      LAVAENGINE_API
      virtual ~Switch( void );

      LAVAENGINE_API
      virtual void forEachNode( std::function<void( Node* )> cb );
      LAVAENGINE_API
      Node* currentNode( void );

      LAVAENGINE_API
      unsigned int getActiveChild( void ) const;
      LAVAENGINE_API
      void setActiveChild( unsigned int idx );
      LAVAENGINE_API
      void disableAllChildren( void )
      {
        _activeChild = SW_INVALID_CHILD;
      }

      LAVAENGINE_API
      void selectNextNode( void );
      LAVAENGINE_API
      void selectPreviousNode( void );

      enum { SW_INVALID_CHILD = -1 };
    private:
      int _activeChild;
    };
  }
}