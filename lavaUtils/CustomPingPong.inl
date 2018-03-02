template<typename T>
CustomPingPong<T>::CustomPingPong(const T & elem1, const T & elem2)
: _elem1(std::move(elem1))
, _elem2(std::move(elem2))
{
}
template<typename T>
void CustomPingPong<T>::swap( void )
{
  std::swap( _elem1, _elem2 );
}
template<typename T>
void CustomPingPong<T>::swap(std::function<void()> cb)
{
  std::swap(_elem1, _elem2);
  if (cb)
  {
    cb();
  }
}
template<typename T>
T CustomPingPong<T>::first( void ) const
{
  return _elem1;
}
template<typename T>
T CustomPingPong<T>::last( void ) const
{
  return _elem2;
}
