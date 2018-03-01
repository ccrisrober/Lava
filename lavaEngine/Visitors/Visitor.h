#ifndef __LAVAENGINE_VISITOR__
#define __LAVAENGINE_VISITOR__

#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class Node;
    class Group;
    class Light;
    class Camera;
    class Geometry;

    class Visitor
    {
    protected:
      Visitor( void ) { }
    public:
      LAVAENGINE_API
      virtual ~Visitor( void );
      LAVAENGINE_API
      virtual void reset( void );
      LAVAENGINE_API
      virtual void traverse( Node* node );
      LAVAENGINE_API
      virtual void visitNode( Node *node );
      LAVAENGINE_API
      virtual void visitGroup( Group *group );
      LAVAENGINE_API
      virtual void visitCamera( Camera *camera );
      LAVAENGINE_API
      virtual void visitLight( Light* light );
      LAVAENGINE_API
      virtual void visitGeometry( Geometry *geometry );
    private:
      Visitor( const Visitor& );
      Visitor &operator= ( const Visitor& );
    };
  }
}

#endif /* __LAVAENGINE_VISITOR__ */
