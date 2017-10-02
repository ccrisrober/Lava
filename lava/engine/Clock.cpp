#include "Clock.hpp"

#include <chrono>

namespace lava
{
  namespace engine
  {
    Clock::Clock( void )
    {
    	reset( );
    }

    Clock::Clock( double dt )
    {
      reset( );

      _deltaTime = dt;
    }

    Clock::Clock( const Clock &t )
    {
    	_currentTime = t._currentTime;
    	_lastTime = t._lastTime;
    	_deltaTime = t._deltaTime;
      _accumTime = t._accumTime;
    }

    Clock::~Clock( void )
    {
    }

    Clock &Clock::operator=( const Clock &t )
    {
    	_currentTime = t._currentTime;
    	_lastTime = t._lastTime;
    	_deltaTime = t._deltaTime;
      _accumTime = t._accumTime;

    	return *this;
    }

    void Clock::reset( void )
    {
      auto now = std::chrono::high_resolution_clock::now( )
        .time_since_epoch( );

      _currentTime = 0.001 * std::chrono::duration_cast<
        std::chrono::milliseconds >( now ).count( );
      _lastTime = _currentTime;
      _deltaTime = 0.0;
      _accumTime = 0.0;
    }

    void Clock::tick( void )
    {
      auto now = std::chrono::high_resolution_clock::now( )
        .time_since_epoch( );

      _currentTime = 0.001 * std::chrono::duration_cast<
        std::chrono::milliseconds >( now ).count( );
      _deltaTime = _currentTime - _lastTime;
      _lastTime = _currentTime;
      _accumTime += _deltaTime;

      onTick( );
    }

    double Clock::getCurrentTime( void ) const
    {
      return _currentTime;
    }
    double Clock::getLastTime( void ) const
    {
      return _lastTime;
    }
    double Clock::getDeltaTime( void ) const
    {
      return _deltaTime;
    }
    void Clock::setTimeout( Clock::TimeoutCallback const &callback,
      double timeout )
    {
      _timeoutCallback = callback;
      _timeout = timeout;
      _repeat = false;
    }

    void Clock::setInterval( Clock::TimeoutCallback const &callback,
      double timeout )
    {
      _timeoutCallback = callback;
      _timeout = timeout;
      _repeat = true;
    }

    Clock &Clock::operator+=( float deltaTime )
    {
      _deltaTime = deltaTime;
      _accumTime += _deltaTime;
      onTick( );
      return *this;
    }

    Clock &Clock::operator+=( const Clock &other )
    {
      *this += other.getDeltaTime( );
      return *this;
    }

    void Clock::onTick( void )
    {
    	if ( _timeoutCallback != nullptr )
      {
    		_timeout -= _deltaTime;
    		if ( _timeout <= 0.0 )
        {
    			_timeoutCallback( );
    			if ( !_repeat )
          {
    				_timeoutCallback = nullptr;
    			}
    		}
    	}
    }
  }
}
