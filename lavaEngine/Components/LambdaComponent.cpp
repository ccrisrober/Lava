#include "LambdaComponent.hpp"

namespace mb
{
  LambdaComponent::LambdaComponent( LambdaCallback cb )
    : _callback( cb )
  {
  }
  void LambdaComponent::update( const mb::Clock& clock )
  {
    _callback( node(), clock );
  }
}
