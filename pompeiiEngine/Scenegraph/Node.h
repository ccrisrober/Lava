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

#ifndef __POMPEIIENGINE_NODE__
#define __POMPEIIENGINE_NODE__

#include "../glm_config.h"

#include <pompeiiEngine/api.h>

#include <pompeiiEngine/Visitors/Visitor.h>
#include <pompeiiEngine/Utils/Layer.h>

#ifdef POMPEIIENGINE_HASCOMPONENTS
  #include <unordered_map>
  #include <functional>
  #include <pompeiiEngine/Components/Component.h>
#endif

#include <vector>

namespace pompeii
{
	namespace engine
	{
    class Node
    {
    public:
      enum class TransformSpace: short
      {
        Local, Parent, World
      };
      //POMPEIIENGINE_API
      //Node( void );
      POMPEIIENGINE_API
      Node( const std::string& name );
      POMPEIIENGINE_API
      virtual ~Node( void );
      POMPEIIENGINE_API
      std::string name( void ) const;
      POMPEIIENGINE_API
      void name( const std::string& name );
    protected:
      Node* _parent;
      std::string _name;

    public:
      std::string tag;
      POMPEIIENGINE_API
      void perform( Visitor &visitor );
      POMPEIIENGINE_API
      void perform( const Visitor &visitor );
      POMPEIIENGINE_API
      virtual void accept( Visitor &visitor );
    public:
#ifdef POMPEIIENGINE_HASCOMPONENTS
      POMPEIIENGINE_API
      void startComponents( void );
      POMPEIIENGINE_API
      void addComponent( Component* comp );
      POMPEIIENGINE_API
      void updateComponents( const pompeii::engine::Clock& clock );
      POMPEIIENGINE_API
      void detachAllComponents( void );
      POMPEIIENGINE_API
      void forEachComponent( std::function< void( Component * ) > callback );

      template< class T, typename ... Args >
      T* addComponent( Args&& ... args );
      template <class T>
      bool hasComponent( void );
      template <class T>
      T* getComponent( void );
      template <class T>
      void removeComponent( void );
      template <class T>
      void removeComponents( void );
      template <class T>
      T* componentInParent( void );
      /** TODO: More functions!
        - GetComponentsInParent<T>(bool includeInactives)
        - GetComponentsInChildren<T>(bool includeInactives)
        - FindNodesWithTag(string tag) NOTE: Global? Local?
      */

      POMPEIIENGINE_API
      Component* getComponentByName( const std::string& name );
      POMPEIIENGINE_API
      std::vector<Component*> getComponentsByName( const std::string& name,
        bool includeInactive = false );
    protected:
      std::unordered_map< std::string, Component* > _components;
#endif
    public:
      POMPEIIENGINE_API
      inline bool hasParent( void ) const
      {
        return _parent != nullptr;
      }
      POMPEIIENGINE_API
      Node* parent( void );

      template< class NodeType >
      const Node* parent( void )
      {
        return static_cast< NodeType* >( _parent );
      }

      template<class NodeClass>
      NodeClass* parent( void );

      void parent( Node* p );
    public:
      POMPEIIENGINE_API
      virtual void setEnabled( bool enabled )
      {
        _enabled = enabled;
      }
      POMPEIIENGINE_API
      bool isEnabled( void )
      {
        return _enabled;
      }

    private:
      bool _enabled = true;

    public:
      POMPEIIENGINE_API
      Layer& layer( void )
      {
        return _layer;
      }
    protected:
      Layer _layer;
    // Transforms section
    public:
      POMPEIIENGINE_API
      void translate( const glm::vec3& direction,
        TransformSpace space = TransformSpace::Local );
      POMPEIIENGINE_API
      void rotate( float angle, const glm::vec3& axis, 
        TransformSpace space = TransformSpace::Local );
      POMPEIIENGINE_API
      void rotate( const glm::quat& quat,
        TransformSpace space = TransformSpace::Local );
      POMPEIIENGINE_API
      void scale( const glm::vec3& scale );
      POMPEIIENGINE_API
      void setPosition( const glm::vec3& position,
        TransformSpace space = TransformSpace::Local );
      POMPEIIENGINE_API
      void setRotation( float angle, const glm::vec3& axis, 
        TransformSpace space = TransformSpace::Local );
      POMPEIIENGINE_API
      void setRotation( const glm::quat& rotation, 
        TransformSpace space = TransformSpace::Local );
      POMPEIIENGINE_API
      void setDirection( const glm::vec3& spaceTargetDirection, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector,
        TransformSpace space = TransformSpace::Local );
      POMPEIIENGINE_API
      void lookAt( const glm::vec3& targetPosition, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector,
        TransformSpace space = TransformSpace::Local );
      POMPEIIENGINE_API
      void update( void );
      POMPEIIENGINE_API
      inline glm::vec3& getAbsolutePosition( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absolutePosition;
      }

      POMPEIIENGINE_API
      inline glm::quat& getAbsoluteRotation( void )
      {
        if ( _needUpdate )
        {
          update( );
        }
        return _absoluteRotation;
      }

      POMPEIIENGINE_API
      inline glm::vec3& getAbsoluteScale( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absoluteScale;
      }

      POMPEIIENGINE_API
      inline glm::mat4& getTransform( void )
      {
        if ( _needUpdate )
        {
          update( );
        }
        return _transform;
      }
    public:
      virtual void needUpdate( void );  // TODO
    private:
      glm::vec3 _position = glm::vec3( 0.0f );
      glm::quat _rotation;// = glm::quat( 0.0f, 0.0f, 0.0f, 1.0f );
      glm::vec3 _scale = glm::vec3( 1.0f );

      glm::vec3 _absolutePosition = glm::vec3( 0.0f );
      glm::quat _absoluteRotation;// = glm::quat( 0.0f, 0.0f, 0.0f, 1.0f );
      glm::vec3 _absoluteScale = glm::vec3( 1.0f );

      glm::mat4 _transform = glm::mat4( 1.0f );

      // TODO OPTIMIZATION: If identity, discard multiplication on global matrices generation!! bool _isIdentity;

      bool _needUpdate = true;
    };
#ifdef POMPEIIENGINE_HASCOMPONENTS
  #include "Node.inl"
#endif
	}
}

#endif /* __POMPEIIENGINE_NODE__ */