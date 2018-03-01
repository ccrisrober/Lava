#include "StartComponents.h"
#include <lavaEngine/Scenegraph/Node.h>

namespace lava
{
	namespace engine
	{
		StartComponents::StartComponents( void )
		: LambdaVisitor( [] ( Node*n ){ n->startComponents( ); } )
		{
		}
	}
}
