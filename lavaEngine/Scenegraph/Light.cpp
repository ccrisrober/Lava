#include "Light.h"
#include <iostream>

namespace lava
{
	namespace engine
	{
		Light::Light( Light::Type t )
			: Node( std::string( "Light" ) )
			, _type( t )
			, _diffuseColor( Color( 1.0f, 1.0f, 1.0f ) )
			, _ambientColor( Color( 0.0f, 0.0f, 0.0f ) )
			, _groundColor( Color( 0.0f, 0.0f, 0.0f ) )
      , _shadowType( ShadowType::NONE )
		{
		}
		Light::~Light( void )
		{
    		std::cout << "[D] Light '" << this->name( ) << "'" << std::endl;
		}

		void Light::accept( Visitor& visitor )
		{
			visitor.visitLight( this );
		}
	}
}