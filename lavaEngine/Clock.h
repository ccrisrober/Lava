/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __LAVA_ENGINE_CLOCK__
#define __LAVA_ENGINE_CLOCK__

#include <functional>
#include <lavaEngine/api.h>

namespace lava
{
  namespace engine
  {
    class Clock
    {
    public:
      LAVAENGINE_API
      Clock( void );
      LAVAENGINE_API
      explicit Clock( double dt );
      LAVAENGINE_API
      Clock( const Clock &c );
      LAVAENGINE_API
      ~Clock( void );
      LAVAENGINE_API
      Clock &operator=( const Clock &other );

      LAVAENGINE_API
      double reset( void );
      LAVAENGINE_API
      void tick( void );

      LAVAENGINE_API
      double getCurrentTime( void ) const;
      LAVAENGINE_API
      double getLastTime( void ) const;
      LAVAENGINE_API
      double getDeltaTime( void ) const;
      LAVAENGINE_API
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
      LAVAENGINE_API
      void setTimeout( TimeoutCallback const &callback,
        double timeout );
      LAVAENGINE_API
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
      LAVAENGINE_API
      Clock &operator+=( float delta );

      /**
       Ticks the clock by another clock's delta time
       As a side effect, _accumTime gets incremented by the new
       _deltaTime. Callbacks get executed if timeout is over
       */
      LAVAENGINE_API
      Clock &operator+=( const Clock &other );

    private:
      void onTick( void );
    };
  }
}

#endif /* __LAVA_ENGINE_CLOCK__ */
