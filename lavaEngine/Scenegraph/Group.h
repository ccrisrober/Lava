#ifndef __LAVA_ENGINE_GROUP__
#define __LAVA_ENGINE_GROUP__

#include "Node.h"
#include <vector>
#include <functional>

namespace lava
{
	namespace engine
	{
		class Group: public Node
		{
		public:
      LAVAENGINE_API
      Group( const std::string name );
      LAVAENGINE_API
      virtual ~Group( );

      LAVAENGINE_API
      bool hasNodes( void ) const;
      LAVAENGINE_API
      unsigned int getNumChildren( void ) const;

      LAVAENGINE_API
      virtual void addChild( Node* node );
      LAVAENGINE_API
      virtual void removeChild( Node* node );
      LAVAENGINE_API
      virtual void removeChildren( void );
      LAVAENGINE_API
      Node* nodeAt( unsigned int idx );

      template<typename T>
      T* nodeAt( unsigned int idx )
      {
        return static_cast< T* >( nodeAt( idx ) );
      }
      LAVAENGINE_API
      void insertChild( unsigned int idx, Node* node );
      LAVAENGINE_API
      void removeChild( unsigned int idx );
      LAVAENGINE_API
      virtual void forEachNode( std::function< void( Node * ) > callback );
    protected:
      std::vector<Node*> _children;

    public:
      LAVAENGINE_API
      virtual void accept( Visitor& v );

    public:
      virtual void needUpdate( void ) override;
		};
	}
}

#endif /* __LAVA_ENGINE_GROUP__ */