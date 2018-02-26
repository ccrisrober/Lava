#include "Scene.h"

namespace lava
{
  namespace engine
  {
    Scene::Scene( Node* root = nullptr )
      : _root ( root )
    {
    }
    Node* Scene::getRoot( void ) const
    {
      return _root;
    }
    void Scene::setRoot( Node* root )
    {
      this->_root = root;
    }
  }
}