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

#ifndef __LAVAUTILS_NODE__
#define __LAVAUTILS_NODE__

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <lavaUtils/api.h>

namespace lava
{
	namespace utility
	{
		class Node
		{
		public:
			enum class TransformSpace: short
			{
				Local, Parent, World
			};
			
			LAVAUTILS_API
			Node( const std::string& name );
			//LAVAUTILS_API
			//virtual ~Node( void ) = default;
			LAVAUTILS_API
			Node* node( const std::string& name );
			LAVAUTILS_API
			const Node* getNode( const std::string& name ) const;

			LAVAUTILS_API
			void setParent( Node *parent );
			LAVAUTILS_API
			Node* getParent( void ) const;
			LAVAUTILS_API
			const std::string& getName( void ) const;
			LAVAUTILS_API
			void addChild( Node& child );

			LAVAUTILS_API
			void translate( const glm::vec3& d, 
				TransformSpace space = TransformSpace::Local );
			LAVAUTILS_API
			void translate( float x, float y, float z,
				TransformSpace space = TransformSpace::Local );
		};
	}
}


#endif /* __LAVAUTILS_NODE__ */