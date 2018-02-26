#ifndef __LAVAENGINE_NODE__
#define __LAVAENGINE_NODE__

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include <lavaEngine/api.h>

#include <lavaEngine/Visitors/Visitor.h>

#ifdef LAVAENGINE_HASCOMPONENTS
  #include <unordered_map>
#endif

namespace lava
{
	namespace engine
	{
    class Node
    {
    public:
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
      void detachAllComponents( void );
      LAVAENGINE_API
      void forEachComponent( std::function< void( Component * ) > callback );
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
      void setEnabled( bool enabled )
      {
        _enabled = enabled;
      }
      bool isEnabled( void )
      {
        return _enabled;
      }

    private:
      bool _enabled = true;
    };
	}
}


#endif /* __LAVAENGINE_NODE__ */