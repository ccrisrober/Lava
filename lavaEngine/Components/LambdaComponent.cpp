#include "LambdaComponent.h"

namespace lava
{
	namespace engine
	{
		LambdaComponent::LambdaComponent( LambdaCallback cb )
			: _callback( cb )
		{
		}
		/*void LambdaComponent::update( const mb::Clock& clock )
		{
			_callback( node( ), clock );
		}*/
	}
}
