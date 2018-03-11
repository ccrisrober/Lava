#ifndef __LAVAUTILS_CUSTOM_PING_PONG__
#define __LAVAUTILS_CUSTOM_PING_PONG__

#include <lavaUtils/api.h>

#include <functional>

namespace lava
{
  namespace utility
  {
    template <class T>
    class CustomPingPong
    {
    public:
      CustomPingPong( const T & elem1, const T & elem2 )
        : _elem1( std::move( elem1 ) )
        , _elem2( std::move( elem2 ) )
      {
      }
      void swap( void )
      {
        std::swap( _elem1, _elem2 );
      }
      void swap( std::function<void( )> cb )
      {
        swap( );
        if ( cb )
        {
          cb( );
        }
      }
      T first( void ) const
      {
        return _elem1;
      }
      T last( void ) const
      {
        return _elem2;
      }
    protected:
      T _elem1;
      T _elem2;
    };
  }
  //#include "CustomPingPong.inl"
}

#endif /* __LAVAUTILS_CUSTOM_PING_PONG__ */
