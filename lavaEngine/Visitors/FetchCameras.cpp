#include "FetchCameras.h"

namespace lava
{
	namespace engine
	{
		void FetchCameras::reset( void )
		{
			_cameras.clear( );
			Visitor::reset( );
		}

		void FetchCameras::visitCamera( Camera *c )
		{
			_cameras.push_back( c );
		}

		void FetchCameras::forEachCameras( std::function< void( Camera* ) > cb )
		{
			for ( auto& camera : _cameras )
			{
				cb( camera );
			}
			_cameras.clear( );
		}
	}
}