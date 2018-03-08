#include "RotateComponent.h"
#include <lavaEngine/Scenegraph/Node.h>
#include <lavaEngine/Mathematics/Mathf.h>

namespace lava
{
  namespace engine
  {
    RotateComponent::RotateComponent( const glm::vec3& axis, float speed )
      : _axis( axis )
      , _speed( speed )
      , _time( 0.0f )
    {
    }
    void RotateComponent::update( const lava::engine::Clock& clock )
    {
      node( )->rotate( _time * 2.0f - Mathf::PI, _axis, 
        Node::TransformSpace::Local );

      //.fromAxisAngle( _axis, _time * 2.0f * mb::Mathf::PI );
      _time += _speed * clock.getDeltaTime( );
    }
  }
}
