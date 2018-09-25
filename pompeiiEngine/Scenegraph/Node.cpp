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
#include <iostream>

namespace pompeii
{
	namespace engine
	{
		Node::Node( const std::string& name )
      : _parent( nullptr )
      , _name ( name )
		{
		}
    Node::~Node( void )
    {
      std::cout << "[D] Node '" << this->name( ) << "'" << std::endl;
#ifdef POMPEIIENGINE_HASCOMPONENTS
      detachAllComponents( );
#endif
    }
    std::string Node::name( void ) const
    {
      return _name;
    }
    void Node::name( const std::string & name )
    {
      _name = name;
    }
    Node* Node::parent( void )
    {
      return _parent;
    }
    void Node::parent( Node * p )
    {
      _parent = p;
    }
    void Node::perform( Visitor& visitor )
    {
      visitor.traverse( this );
    }
    void Node::perform( const Visitor& visitor )
    {
      const_cast< Visitor& >( visitor ).traverse( this );
    }
    void Node::accept( Visitor& visitor )
    {
      visitor.visitNode( this );
    }
#ifdef POMPEIIENGINE_HASCOMPONENTS
    void Node::startComponents( void )
    {
      forEachComponent( [ ]( Component* c ) {
        c->start( );
      } );
    }
    void Node::addComponent( Component * comp )
    {
      comp->setNode( this );
      _components.insert( std::pair<std::string, Component*>( comp->GetUID( ), comp ) );
      comp->onAttach( );
    }
    void Node::updateComponents( const pompeii::engine::Clock & clock )
    {
      for ( auto& comp : _components )
      {
        if ( comp.second->isEnabled( ) )
        {
          comp.second->update( clock );
        }
      }
    }
    void Node::detachAllComponents( void )
    {
      forEachComponent( [ ]( Component *cmp )
      {
        cmp->onDetach( );
        cmp->setNode( nullptr );
      } );

      _components.clear( );
    }
    void Node::forEachComponent( std::function<void( Component* )> callback )
    {
      // create a copy of the component's collection
      // to prevent errors when attaching or detaching
      // components during an update pass
      auto cs = _components;
      for ( auto cmp : cs )
      {
        if ( cmp.second != nullptr )
        {
          callback( cmp.second );
        }
      }
    }
    Component* Node::getComponentByName( const std::string & name )
    {
      auto aux = _components.find( name );
      if ( aux == _components.end( ) )
      {
        return nullptr;
      }
      return aux->second;
    }
    std::vector<Component*> Node::getComponentsByName( 
      const std::string & name, bool includeInactive )
    {
      std::vector<Component*> cs;

      auto finds = _components.equal_range( name );

      for ( auto element = finds.first; element != finds.second; ++element )
      {
        if ( includeInactive || element->second->isEnabled( ) )
        {
          cs.push_back( element->second );
        }
      }
      return cs;
    }
#endif

    

    void Node::translate( const glm::vec3& direction,
      TransformSpace space )
    {
      if ( space == TransformSpace::Local )
      {
        _position += glm::toMat3( _rotation ) * direction;
      }
      else if ( space == TransformSpace::Parent )
      {
        _position += direction;
      }
      else if ( space == TransformSpace::World )
      {
        if ( _parent )
        {
          _position += ( glm::inverse( 
            glm::toMat3( _parent->getAbsoluteRotation( ) )
          ) * direction ) / _parent->getAbsoluteScale( );
        }
        else
        {
          _position += direction;
        }
      }

      needUpdate( );
    }
    void Node::rotate( float angle, const glm::vec3& axis, 
      TransformSpace space )
    {
      rotate( glm::quat( angle, axis ), space );
    }
    void Node::rotate( const glm::quat& quat,
      TransformSpace space )
    {
      if ( space == TransformSpace::Local ) {
        _rotation = _rotation * quat;
      }
      else if ( space == TransformSpace::Parent ) {
        _rotation = quat * _rotation;
      }
      else if ( space == TransformSpace::World ) {
        _rotation = _rotation * glm::inverse( 
          getAbsoluteRotation( ) ) * quat * getAbsoluteRotation( );
      }

      needUpdate( );
    }
    void Node::scale( const glm::vec3& scale )
    {
      _scale *= scale;
      needUpdate( );
    }
    void Node::setPosition( const glm::vec3& position,
      TransformSpace space )
    {
      if ( space == TransformSpace::Local )
      {
        _position = glm::toMat3( _rotation ) * position;
      }
      else if ( space == TransformSpace::Parent )
      {
        _position = position;
      }
      else if ( space == TransformSpace::World )
      {
        if ( _parent )
        {
          _position = ( glm::toMat3( _parent->getAbsoluteRotation( ) ) *
            position * _parent->getAbsoluteScale( ) ) + 
            _parent->getAbsolutePosition( );
        }
        else {
          _position = position;
        }
      }

      needUpdate( );
    }
    void Node::setRotation( float angle, const glm::vec3& axis, 
      TransformSpace space )
    {
      setRotation( glm::quat( angle, axis ), space );
    }
    void Node::setRotation( const glm::quat& rotation, 
      TransformSpace space )
    {
      if ( space == TransformSpace::Local )
      {
        _rotation = rotation;
      }
      else if ( space == TransformSpace::Parent )
      {
        _rotation = rotation;
      }
      else if ( space == TransformSpace::World )
      {
        if ( _parent )
        {
          _rotation = glm::inverse( getAbsoluteRotation( ) ) * rotation;
        }
        else
        {
          _rotation = rotation;
        }
      }

      needUpdate( );
    }

    void Node::setDirection( const glm::vec3& spaceTargetDirection, 
      const glm::vec3& localDirectionVector, 
      const glm::vec3& localUpVector,
      TransformSpace space )
    {
      // The direction we want the local direction point to
      glm::vec3 targetDirection = glm::normalize( spaceTargetDirection );

      // Transform target direction to world space
      if ( space == TransformSpace::Local )
      {
        targetDirection = glm::toMat3(getAbsoluteRotation( ) ) * 
        targetDirection;
      }
      else if ( space == TransformSpace::Parent )
      {
        if ( _parent )
        {
          targetDirection = glm::toMat3( _parent->getAbsoluteRotation( ) ) 
            * targetDirection;
        }
      }
      else if ( space == TransformSpace::World )
      {
        // Nothing to do here
      }

      glm::vec3 xVec = glm::normalize( cross( localUpVector, targetDirection ) );
      glm::vec3 yVec = glm::normalize( cross( targetDirection, xVec ) );

      glm::mat4 rotMatrix( 1.0f );
      rotMatrix[ 0 ][ 0 ] = xVec.x;
      rotMatrix[ 1 ][ 0 ] = xVec.y;
      rotMatrix[ 2 ][ 0 ] = xVec.z;

      rotMatrix[ 0 ][ 1 ] = yVec.x;
      rotMatrix[ 1 ][ 1 ] = yVec.y;
      rotMatrix[ 2 ][ 1 ] = yVec.z;

      rotMatrix[ 0 ][ 2 ] = targetDirection.x;
      rotMatrix[ 1 ][ 2 ] = targetDirection.y;
      rotMatrix[ 2 ][ 2 ] = targetDirection.z;

      glm::quat unitZToTarget = glm::quat( rotMatrix );

      glm::quat targetOrientation;

      if ( localDirectionVector == glm::vec3{ 0.0f, 0.0f, -1.0f } )
      {
        targetOrientation = glm::quat( -unitZToTarget.y, 
          -unitZToTarget.z, unitZToTarget.w, unitZToTarget.x );
      }
      else
      {
        //targetOrientation = unitZToTarget * directionTo( localDirectionVector, 
        //  glm::vec3{ 0.0f, 0.0f, 1.0f } );
      }

      setRotation( targetOrientation, TransformSpace::Parent );
    }
    void Node::lookAt( const glm::vec3& targetPosition, 
      const glm::vec3& localDirectionVector, 
      const glm::vec3& localUpVector,
      TransformSpace space )
    {
      glm::vec3 origin;
      if ( space == TransformSpace::Local )
      {
        origin = glm::vec3( 0.0f );
      }
      else if ( space == TransformSpace::Parent )
      {
        origin = _position;
      }
      else if ( space == TransformSpace::World )
      {
        origin = getAbsolutePosition( );
      }
      
      setDirection( targetPosition - origin, localDirectionVector, 
        localUpVector );
    }

    void Node::update( void )
    {
      if ( _parent )
      {
        glm::mat3 auxRotation = glm::toMat3( _parent->getAbsoluteRotation( ) );
        _absolutePosition = ( _parent->getAbsoluteScale( ) * _position );
        _absolutePosition += _parent->getAbsolutePosition( );

        _absolutePosition = auxRotation * _absolutePosition;

        _absoluteRotation = _parent->getAbsoluteRotation( ) * _rotation;
        _absoluteScale = _parent->getAbsoluteScale( ) * _scale;
      }
      else
      {
        _absolutePosition = _position;
        _absoluteRotation = _rotation;
        _absoluteScale = _scale;
      }

      _absoluteRotation = glm::normalize( _absoluteRotation );
      
      _transform = glm::mat4( 1.0f );
      _transform = glm::translate( _transform, _absolutePosition );
      glm::mat4 auxRotation = glm::toMat4( _absoluteRotation );
      _transform = _transform * auxRotation;
      _transform = glm::scale( _transform, _absoluteScale );

      _needUpdate = false;
    }

    void Node::needUpdate( void )
    {
      _needUpdate = true;
      /*for( auto& c: _children )
      {
        c->needUpdate( );
      }*/
    }
	}
}