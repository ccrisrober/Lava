#ifndef __LAVA_ENGINE_NODE__
#define __LAVA_ENGINE_NODE__

#include <lava/api.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace lava
{
  namespace engine
  {
    class Node
    {
    public:
      Node( const std::string& name )
        : _name ( name )
      {
      }

      Node( const Node& ) = delete;
      Node( Node&& ) = delete;

      Node& operator=( const Node& ) = delete;
      Node& operator=( Node& ) = delete;

      virtual ~Node( void ) = default;

      void setParent( Node *parent )
      {
        _parent = parent;
        needUpdate( );
      }
      Node* getParent( void ) const
      {
        return _parent;
      }

      const std::string& getName( void ) const
      {
        return _name;
      }

      void addChild( Node& child )
      {
        child._parent = this;
        _children.push_back( &child );
      }

      void translate( const glm::vec3& dir )
      {
        // Local space
        _position += glm::mat3( _rotation ) * dir;

        needUpdate( );
      }
      void rotate( float angle, const glm::vec3& axis )
      {
        rotate( glm::quat( angle, axis ) );
      }
      void rotate( const glm::quat& quat )
      {
        // Local space
        _rotation = _rotation * quat;

        needUpdate( );
      }
      void scale( const glm::vec3& scale )
      {
        _scale *= scale;
        needUpdate( );
      }
      void setPosition( const glm::vec3& position )
      {
        // Local space
        _position = glm::mat3( _rotation ) * position;

        needUpdate( );
      }
      void setRotation( float angle, const glm::vec3& axis )
      {
        setRotation( glm::quat( angle, axis ) );
      }
      void setRotation( const glm::quat& rotation )
      {
        // Local space
        _rotation = rotation;

        needUpdate( );
      }

      void setDirection( const glm::vec3& spaceTargetDirection, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector )
      {
        /*glm::vec3 targetDir = glm::normalize( spaceTargetDirection );

        // Local space
        targetDir = glm::mat3( getAbsoluteRotation( ) ) * targetDir;

        glm::vec3 x = glm::normalize( glm::cross( localUpVector, targetDir ) );
        glm::vec3 y = glm::normalize( glm::cross( targetDir, x ) );
        glm::vec3 z = glm::quat::*/
      }
      void lookAt( const glm::vec3& targetPosition, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector )
      {
        glm::vec3 origin;
        // Local space
        origin = _position;
        
        setDirection( targetPosition - origin, localDirectionVector, localUpVector );
      }

      void update( void )
      {
        // todo: Create final transform

        if ( _parent )
        {
          //_absolutePosition = glm::mat3(_parent->getAbsoluteRotation( ) ) * 
          //  ( _parent->getAbsoluteScale( ) * _position ) + _parent->getAbsolutePosition( );
          //_absoluteRotation = _parent->getAbsoluteRotation( ) * _rotation;
          //_absoluteScale = _parent->getAbsoluteScale( ) * _scale;
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
        //_transform = glm::rotate( _transform, _absoluteRotation );
        _transform = glm::scale( _transform, _absoluteScale );

        _needUpdate = false;
      }
      inline const glm::vec3& Node::getAbsolutePosition( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absolutePosition;
      }

      inline const glm::quat& Node::getAbsoluteRotation( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absoluteRotation;
      }

      inline const glm::vec3& Node::getAbsoluteScale( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absoluteScale;
      }

      inline const glm::mat4& Node::getTransform( void )
      {
        if ( _needUpdate )
        {
          update( );
        }
        return _transform;
      }
    protected:
      Node* _parent = nullptr;
      std::string _name = "";
      std::vector< Node* > _children;

    private:
      void needUpdate( void )
      {
        _needUpdate = true;
        for( auto& c: _children )
        {
          c->needUpdate( );
        }
      }
      glm::vec3 _position;
      glm::quat _rotation;
      glm::vec3 _scale;

      glm::vec3 _absolutePosition;
      glm::quat _absoluteRotation;
      glm::vec3 _absoluteScale;

      glm::mat4 _transform;

      bool _needUpdate = true;
    };
  }
}

#endif /* __LAVA_ENGINE_NODE__ */