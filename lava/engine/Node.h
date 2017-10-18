#ifndef LAVA_ENGINE_NODE
#define LAVA_ENGINE_NODE

#include <lava/api.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

namespace lava
{
  namespace engine
  {
    class Node
    {
    public:
      enum class TransformSpace : short
      {
        Local,
        Parent,
        World
      };
      Node( const std::string& name )
        : _name ( name )
      {
        _scale = glm::vec3( 1.0f );
      }

      Node( const Node& ) = delete;
      Node( Node&& ) = delete;

      Node& operator=( const Node& ) = delete;
      Node& operator=( Node& ) = delete;

      virtual ~Node( void ) = default;

      Node* getNode( const std::string& name )
      {
        if ( name == _name )
        {
          return this;
        }

        for ( auto& child : _children )
        {
          Node* n = child->getNode( name );

          if ( n )
          {
            return n;
          }
        }

        return nullptr;
      }

      const Node* getNode( const std::string& name ) const
      {
        if ( name == _name )
        {
          return this;
        }

        for ( const auto& child : _children )
        {
          const Node* n = child->getNode( name );

          if ( n )
          {
            return n;
          }
        }

        return nullptr;
      }

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

      void translate( const glm::vec3& direction,
        TransformSpace space = TransformSpace::Local )
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
      void rotate( float angle, const glm::vec3& axis, 
        TransformSpace space = TransformSpace::Local )
      {
        rotate( glm::quat( angle, axis ), space );
      }
      void rotate( const glm::quat& quat,
        TransformSpace space = TransformSpace::Local )
      {
        if ( space == TransformSpace::Local ) {
          _rotation = _rotation * quat;
        }
        else if ( space == TransformSpace::Parent ) {
          _rotation = quat * _rotation;
        }
        else if ( space == TransformSpace::World ) {
          _rotation = _rotation * glm::inverse( getAbsoluteRotation( ) ) * quat * getAbsoluteRotation( );
        }

        needUpdate( );
      }
      void scale( const glm::vec3& scale )
      {
        _scale *= scale;
        needUpdate( );
      }
      void setPosition( const glm::vec3& position,
        TransformSpace space = TransformSpace::Local )
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
              position * _parent->getAbsoluteScale( ) ) + _parent->getAbsolutePosition( );
          }
          else {
            _position = position;
          }
        }

        needUpdate( );
      }
      void setRotation( float angle, const glm::vec3& axis, 
        TransformSpace space = TransformSpace::Local )
      {
        setRotation( glm::quat( angle, axis ) );
      }
      void setRotation( const glm::quat& rotation, 
        TransformSpace space = TransformSpace::Local )
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

      void setDirection( const glm::vec3& spaceTargetDirection, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector,
        TransformSpace space = TransformSpace::Local )
      {
        // The direction we want the local direction point to
        glm::vec3 targetDirection = glm::normalize( spaceTargetDirection );

        // Transform target direction to world space
        if ( space == TransformSpace::Local )
        {
          targetDirection = glm::toMat3(getAbsoluteRotation( ) ) * targetDirection;
        }
        else if ( space == TransformSpace::Parent )
        {
          if ( _parent )
          {
            targetDirection = glm::toMat3( _parent->getAbsoluteRotation( ) ) * targetDirection;
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
      void lookAt( const glm::vec3& targetPosition, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector,
        TransformSpace space = TransformSpace::Local )
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
        
        setDirection( targetPosition - origin, localDirectionVector, localUpVector );
      }

      void update( void )
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
      inline const glm::vec3& getAbsolutePosition( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absolutePosition;
      }

      inline const glm::quat& getAbsoluteRotation( void )
      {
        if ( _needUpdate )
        {
          update( );
        }
        return _absoluteRotation;
      }

      inline const glm::vec3& getAbsoluteScale( void )
      {
        if ( _needUpdate )
        {
          update( );
        }

        return _absoluteScale;
      }

      inline const glm::mat4& getTransform( void )
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
    class Scene
    {
    public:
      Scene( void ) = default;

      Scene( const Scene& ) = delete;
      Scene( Scene&& ) = delete;

      Scene& operator=( const Scene& ) = delete;
      Scene& operator=( Scene&& ) = delete;

      ~Scene( void ) = default;

      Node& getRoot( void )
      {
        return _root;
      }
      const Node& getRoot( void ) const
      {
        return _root;
      }
    private:
      Node _root;
    };
  }
}

#endif /* LAVA_ENGINE_NODE */