#ifndef __LAVAENGINE_ROTATE_COMPONENT__
#define __LAVAENGINE_ROTATE_COMPONENT__

#include "Component.h"
#include <lavaEngine/api.h>

#include <glm/glm.hpp>

namespace lava
{
  namespace engine
  {
    class RotateComponent : public Component
    {
      IMPLEMENT_COMPONENT( RotateComponent )
    public:
      LAVAENGINE_API
  	  RotateComponent( const glm::vec3& axis, float speed );
      LAVAENGINE_API
    	virtual void update( const lava::engine::Clock& clock ) override;
  	protected:
  	  glm::vec3 _axis;
  	  float _speed;
  	  float _time;
    };
  }
}

#endif /* __LAVAENGINE_ROTATE_COMPONENT__ */
