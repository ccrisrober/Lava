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

#include "Node.h"

namespace lava
{
	namespace utility
	{
		Node::Node( const std::string& name )
		: _parent( nullptr )
		, _name ( name )
		{
			_scale = glm::vec3( 1.0f );
		}
		Node* Node::node( const std::string& name )
		{
			if ( name == _name )
			{
				return this;
			}
			Node* n;
			for( auto& child: _children )
			{
				n = child->node( name );
				if( n )
				{
					return n;
				}
			}
			return nullptr;
		}
		const Node* Node::getNode( const std::string& name ) const
		{
			if ( name == _name )
			{
				return this;
			}
			Node* n;
			for( auto& child: _children )
			{
				n = child->getNode( name );
				if( n )
				{
					return n;
				}
			}
			return nullptr;
		}

		void Node::setParent( Node *parent )
		{
			_parent = parent;
			needUpdate( );
		}
		Node* Node::getParent( void ) const
		{
			return _parent;
		}
		const std::string& Node::getName( void ) const
		{
			return _name;
		}
		void Node::addChild( Node& child )
		{
			child._parent = this;
			_children.push_back( &child );
		}

		void Node::translate( const glm::vec3& d,  TransformSpace space )
		{
			switch( space )
			{
				case TransformSpace::Local:
		            // position is relative to parent so transform downwards
		            //_position += mOrientation * d;
					break;
				case TransformSpace::Parent:
					_position += direction;
					break;
				case TransformSpace::World:
					break;
			}
		}
		void Node::translate( float x, float y, float z, TransformSpace space )
		{
			translate( glm::vec3( x, y, z ), space );
		}
	}
}