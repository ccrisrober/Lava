#ifndef __LAVAENGINE_DUMP_VISITOR__
#define __LAVAENGINE_DUMP_VISITOR__

#include "Visitor.h"
#include <string>

#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class DumpVisitor :
      public Visitor
    {
    public:
      LAVAENGINE_API
      virtual void traverse( Node *node ) override;
      LAVAENGINE_API
      virtual void visitNode( Node *node ) override;
      LAVAENGINE_API
      virtual void visitGroup( Group *group ) override;
      //LAVAENGINE_API
      //virtual void visitGeometry( Geometry *geometry ) override;
      //LAVAENGINE_API
      //virtual void visitCamera( Camera *camera ) override;
      LAVAENGINE_API
      virtual void visitLight( Light *light ) override;
    private:
      void _dumpNode( Node *node, const std::string& type );
      unsigned int _auxLevel = 0;
    };
  }
}

#endif /* __LAVAENGINE_DUMP_VISITOR__ */
