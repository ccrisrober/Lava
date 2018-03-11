#ifndef __LAVA_ENGINE_GEOMETRY__
#define __LAVA_ENGINE_GEOMETRY__

#include "Node.h"
#include <memory>

namespace lava
{
	namespace engine
	{
    class Primitive
    {

    };
		class Geometry : public Node
		{
		public:
			LAVAENGINE_API
			Geometry( const std::string& name = "Geometry" );
			LAVAENGINE_API
			virtual ~Geometry( void );
			LAVAENGINE_API
			void addPrimitive( std::shared_ptr< Primitive > p );
			LAVAENGINE_API
			bool hasPrimitives( void ) const;
			LAVAENGINE_API
			void removePrmitive( std::shared_ptr< Primitive > p );
			LAVAENGINE_API
			void removeAllPrimitives( void );
			LAVAENGINE_API
			void forEachPrimitive( std::function<void(
        std::shared_ptr<Primitive> )> callback );

		protected:
			std::vector< std::shared_ptr< Primitive > > _primitives;
		public:
			virtual void accept( Visitor& v ) override;
		};
	}
}

#endif /* __LAVA_ENGINE_GEOMETRY__ */