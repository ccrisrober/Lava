#include "UpdateComponents.h"
#include <lavaEngine/Scenegraph/Node.h>

namespace lava
{
	namespace engine
	{
	  UpdateComponents::UpdateComponents( const lava::engine::Clock& clock )
	  : LambdaVisitor( [ clock ] ( Node*n ) { n->updateComponents( clock ); } )
	  {
	  }
	}
}
