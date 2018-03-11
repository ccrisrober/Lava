#ifndef __LAVA_ENGINE_COMPUTEBATCHQUEUE__
#define __LAVA_ENGINE_COMPUTEBATCHQUEUE__

#include "Visitor.h"
#include <lavaEngine/api.h>
#include <lavaEngine/Scenegraph/Camera.h>
#include <lavaEngine/Rendering/BatchQueue.h>

#include <memory>

namespace lava
{
  namespace engine
  {
    class ComputeBatchQueue
      : public Visitor
    {
    public:
      LAVAENGINE_API
      ComputeBatchQueue( Camera* camera, std::shared_ptr<BatchQueue> bq );
      LAVAENGINE_API
      virtual void traverse( Node* n ) override;
      LAVAENGINE_API
      virtual void visitGroup( Group* g ) override;
      LAVAENGINE_API
      virtual void visitGeometry( Geometry* g ) override;
      LAVAENGINE_API
      virtual void visitLight( Light* l ) override;
    protected:
      Camera* _camera;
      std::shared_ptr<BatchQueue> _batch;
    };
  }
}

#endif /* __LAVA_ENGINE_COMPUTEBATCHQUEUE__ */