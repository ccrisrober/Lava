template< class T, typename ... Args >
T* Node::addComponent( Args&& ... args )
{
  auto component = new T( std::forward< Args >( args ) ... );
  addComponent( component );
  return component;
}

template<class NodeClass>
NodeClass* Node::parent( void )
{
  return static_cast< NodeClass* >( _parent );
}
template <class T>
bool Node::hasComponent( void )
{
  auto aux = _components.find( T::StaticGetUID( ) );
  if ( aux == _components.end( ) )
  {
    return false;
  }
  return true;
}
template <class T>
T* Node::getComponent( void )
{
  auto aux = _components.find( T::StaticGetUID( ) );
  if ( aux == _components.end( ) )
  {
    return nullptr;
  }
  return static_cast<T*>( aux->second );
}
template <class T>
void Node::removeComponent( void )
{
  auto range = _components.find( T::StaticGetUID( ) );
  auto it = range.first;
  if ( it != range.second )
  {
    it->onDetach( );
    _components.erase( it );
  }
}
template <class T>
void Node::removeComponents( void )
{
  auto range = _components.find( T::StaticGetUID( ) );
  auto it = range.first;
  while ( it != range.second )
  {
    it->onDetach( );
    _components.erase( it++ );
  }
}
template <class T>
T* Node::componentInParent( void )
{
  if ( !hasParent( ) )
  {
    return nullptr;
  }
  return parent( )->getComponent<T>( );
}
