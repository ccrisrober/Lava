#ifndef LAVA_ENGINE_NODE
#define LAVA_ENGINE_NODE

#include <lava/api.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

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
      Node( const std::string& name );
      Node( const Node& ) = delete;
      Node( Node&& ) = delete;

      Node& operator=( const Node& ) = delete;
      Node& operator=( Node& ) = delete;

      virtual ~Node( void ) = default;

      Node* getNode( const std::string& name );

      const Node* getNode( const std::string& name ) const;

      void setParent( Node *parent );
      Node* getParent( void ) const;
      const std::string& getName( void ) const;
      void addChild( Node& child );
      void translate( const glm::vec3& direction,
        TransformSpace space = TransformSpace::Local );
      void rotate( float angle, const glm::vec3& axis, 
        TransformSpace space = TransformSpace::Local );
      void rotate( const glm::quat& quat,
        TransformSpace space = TransformSpace::Local );
      void scale( const glm::vec3& scale );
      void setPosition( const glm::vec3& position,
        TransformSpace space = TransformSpace::Local );
      void setRotation( float angle, const glm::vec3& axis, 
        TransformSpace space = TransformSpace::Local );
      void setRotation( const glm::quat& rotation, 
        TransformSpace space = TransformSpace::Local );
      void setDirection( const glm::vec3& spaceTargetDirection, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector,
        TransformSpace space = TransformSpace::Local );
      void lookAt( const glm::vec3& targetPosition, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector,
        TransformSpace space = TransformSpace::Local );
      void update( void );
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
      Node* _parent;
      std::string _name;
      std::vector< Node* > _children;

    private:
      void needUpdate( void );
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

      Node& getRoot( void );
      const Node& getRoot( void ) const;
    private:
      Node _root;
    };
  }
}

#endif /* LAVA_ENGINE_NODE */