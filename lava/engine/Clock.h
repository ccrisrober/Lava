#ifndef __LAVA_ENGINE_CLOCK__
#define __LAVA_ENGINE_CLOCK__

#include <functional>
#include <lava/api.h>

namespace lava
{
  namespace engine
  {
    class Clock
    {
    public:
      LAVA_API
      Clock( void );
      LAVA_API
      explicit Clock( double dt );
      LAVA_API
      Clock( const Clock &c );
      LAVA_API
      ~Clock( void );
      LAVA_API
      Clock &operator=( const Clock &other );

      LAVA_API
      double reset( void );
      LAVA_API
      void tick( void );

      LAVA_API
      double getCurrentTime( void ) const;
      LAVA_API
      double getLastTime( void ) const;
      LAVA_API
      double getDeltaTime( void ) const;
      LAVA_API
      double getAccumTime( void ) const
      {
        return _accumTime;
      }
    private:
      double _currentTime;
      double _lastTime;
      double _deltaTime;
      double _accumTime;

    public:
      typedef std::function< void( void ) > TimeoutCallback;
      LAVA_API
        void setTimeout( TimeoutCallback const &callback,
        double timeout );
      LAVA_API
        void setInterval( TimeoutCallback const &callback,
        double timeout );

    private:
      TimeoutCallback _timeoutCallback;
      double _timeout;
      bool _repeat;

    public:
      /**
       Ticks the clock by a fixed delta time
       As a side effect, _accumTime gets incremented by the new
       _deltaTime. Callbacks get executed if timeout is over
       */
      LAVA_API
      Clock &operator+=( float delta );

      /**
       Ticks the clock by another clock's delta time
       As a side effect, _accumTime gets incremented by the new
       _deltaTime. Callbacks get executed if timeout is over
       */
      LAVA_API
      Clock &operator+=( const Clock &other );

    private:
      void onTick( void );
    };
  }
}

#endif /* __LAVA_ENGINE_CLOCK__ */
