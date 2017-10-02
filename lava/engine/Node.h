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

      }
      void rotate( float angle, const glm::vec3& axis )
      {

      }
      void rotate( const glm::quat& quat )
      {

      }
      void scale( const glm::vec3& scale )
      {
        _scale *= scale;
        needUpdate( );
      }
      void setPosition( const glm::vec3& position )
      {

      }
      void setRotation( float angle, const glm::vec3& axis )
      {

      }
      void setRotation( const glm::quat& rotation )
      {

      }

      void setDirection( const glm::vec3& spaceTargetDirection, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector )
      {

      }
      void lookAt( const glm::vec3& targetPosition, 
        const glm::vec3& localDirectionVector, 
        const glm::vec3& localUpVector )
      {

      }

      void update( void )
      {
        // todo: Create final transform
        _needUpdate = false;
      }

      const glm::mat4& getTransform( void )
      {
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

      glm::mat4 _transform;

      bool _needUpdate = true;
    };
  }
}

#endif /* __LAVA_ENGINE_NODE__ */