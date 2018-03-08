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