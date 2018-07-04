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

#ifndef __LAVAENGINE_NODE__
#define __LAVAENGINE_NODE__

#include "../glm_config.h"

#include <lavaEngine/api.h>

#include <lavaEngine/Visitors/Visitor.h>
#include <lavaEngine/Utils/Layer.h>

#ifdef LAVAENGINE_HASCOMPONENTS
  #include <unordered_map>
  #include <functional>
  #include <lavaEngine/Components/Component.h>
#endif

#include <vector>

namespace lava
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
      //LAVAENGINE_API
      //Node( void );
      LAVAENGINE_API
      Node( const std::string& name );
      LAVAENGINE_API
      virtual ~Node( void );
      LAVAENGINE_API
      std::string name( void ) const;
      LAVAENGINE_API
      void name( const std::string& name );
    protected:
      Node* _parent;
      std::string _name;

    public:
      std::string tag;
      LAVAENGINE_API
      void perform( Visitor &visitor );
      LAVAENGINE_API
      void perform( const Visitor &visitor );
      LAVAENGINE_API
      virtual void accept( Visitor &visitor );
    public:
#ifdef LAVAENGINE_HASCOMPONENTS
      LAVAENGINE_API
      void startComponents( void );
      LAVAENGINE_API
      void addComponent( Component* comp );
      LAVAENGINE_API
      void updateComponents( const lava::engine::Clock& clock );
      LAVAENGINE_API
      void detachAllComponents( void );
      LAVAENGINE_API
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

      LAVAENGINE_API
      Component* getComponentByName( const std::string& name );
      LAVAENGINE_API
      std::vector<Component*> getComponentsByName( const std::string& name,
        bool includeInactive = false );
    protected:
      std::unordered_map< std::string, Component* > _components;
#endif
    public:
      LAVAENGINE_API
      inline bool hasParent( void ) const
      {
        return _parent != nullptr;
      }
      LAVAENGINE_API
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
      LAVAENGINE_API
      virtual void setEnabled( bool enabled )
      {
        _enabled = enabled;
      }
      LAVAENGINE_API
      bool isEnabled( void )
      {
        return _enabled;
      }

    private:
      bool _enabled = true;

    public:
      LAVAENGINE_API
      Layer& layer( void )
      {
        return _layer;
      }
    protected:
      Layer _layer;
    // Transforms section
    public:
      LAVAENGINE_API
      void translate( const glm::vec3& direction,
        TransformSpace space = TransformSpace::Local );
      LAVAENGINE_API
      void rotate( float angle, const glm::vec3& axis, 
        TransformSpace space = TransformSpace::Local );
      LAVAENGINE_API
      void rotate( const glm::quat& quat,
        TransformSpace space = TransformSpace::Local );
      LAVAENGINE_API
      void scale( const glm::vec3& scale );
      LAVAENGINE_API
      void setPosition( const glm::vec3& position,
        TransformSpace space = TransformSpace::Local );
      LAVAENGINE_API
      void setRotation( float angle, const glm::vec3& axis, 
        TransformSpace space = TransformSpace::Local );
      LAVAENGINE_API
      void setRotation( const glm::quat& rotation, 
        TransformSpace space = TransformSpace::Local );
      LAVAENGINE_API
      void setDirection( const glm::vec3& spaceTargetDirection, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector,
        TransformSpace space = TransformSpace::Local );
      LAVAENGINE_API
      void lookAt( const glm::vec3& targetPosition, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector,
        TransformSpace space = TransformSpace::Local );
      LAVAENGINE_API
      void update( void );
      LAVAENGINE_API
      inline glm::vec3& getAbsolutePosition( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absolutePosition;
      }

      LAVAENGINE_API
      inline glm::quat& getAbsoluteRotation( void )
      {
        if ( _needUpdate )
        {
          update( );
        }
        return _absoluteRotation;
      }

      LAVAENGINE_API
      inline glm::vec3& getAbsoluteScale( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absoluteScale;
      }

      LAVAENGINE_API
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
#ifdef LAVAENGINE_HASCOMPONENTS
  #include "Node.inl"
#endif
	}
}

#endif /* __LAVAENGINE_NODE__ */