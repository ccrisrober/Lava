/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#ifndef __POMPEII_ENGINE_CLOCK__
#define __POMPEII_ENGINE_CLOCK__

#include <functional>
#include <pompeiiEngine/api.h>

namespace pompeii
{
  namespace engine
  {
    class Clock
    {
    public:
      POMPEIIENGINE_API
      Clock( void );
      POMPEIIENGINE_API
      explicit Clock( double dt );
      POMPEIIENGINE_API
      Clock( const Clock &c );
      POMPEIIENGINE_API
      ~Clock( void );
      POMPEIIENGINE_API
      Clock &operator=( const Clock &other );

      POMPEIIENGINE_API
      double reset( void );
      POMPEIIENGINE_API
      void tick( void );

      POMPEIIENGINE_API
      double getCurrentTime( void ) const;
      POMPEIIENGINE_API
      double getLastTime( void ) const;
      POMPEIIENGINE_API
      double getDeltaTime( void ) const;
      POMPEIIENGINE_API
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
      POMPEIIENGINE_API
      void setTimeout( TimeoutCallback const &callback,
        double timeout );
      POMPEIIENGINE_API
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
      POMPEIIENGINE_API
      Clock &operator+=( float delta );

      /**
       Ticks the clock by another clock's delta time
       As a side effect, _accumTime gets incremented by the new
       _deltaTime. Callbacks get executed if timeout is over
       */
      POMPEIIENGINE_API
      Clock &operator+=( const Clock &other );

    private:
      void onTick( void );
    };
  }
}

#endif /* __POMPEII_ENGINE_CLOCK__ */
