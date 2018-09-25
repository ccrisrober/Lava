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

#ifndef __POMPEII_ENGINE_GROUP__
#define __POMPEII_ENGINE_GROUP__

#include "Node.h"
#include <vector>
#include <functional>

namespace pompeii
{
	namespace engine
	{
		class Group: public Node
		{
		public:
      POMPEIIENGINE_API
      Group( const std::string name );
      POMPEIIENGINE_API
      virtual ~Group( );

      POMPEIIENGINE_API
      bool hasNodes( void ) const;
      POMPEIIENGINE_API
      unsigned int getNumChildren( void ) const;

      POMPEIIENGINE_API
      virtual void addChild( Node* node );
      POMPEIIENGINE_API
      virtual void removeChild( Node* node );
      POMPEIIENGINE_API
      virtual void removeChildren( void );
      POMPEIIENGINE_API
      Node* nodeAt( unsigned int idx );

      template<typename T>
      T* nodeAt( unsigned int idx )
      {
        return static_cast< T* >( nodeAt( idx ) );
      }
      POMPEIIENGINE_API
      void insertChild( unsigned int idx, Node* node );
      POMPEIIENGINE_API
      void removeChild( unsigned int idx );
      POMPEIIENGINE_API
      virtual void forEachNode( std::function< void( Node * ) > callback );
    protected:
      std::vector<Node*> _children;

    public:
      POMPEIIENGINE_API
      virtual void accept( Visitor& v );

    public:
      virtual void needUpdate( void ) override;
		};
	}
}

#endif /* __POMPEII_ENGINE_GROUP__ */