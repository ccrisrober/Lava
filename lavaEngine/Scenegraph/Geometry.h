/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

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