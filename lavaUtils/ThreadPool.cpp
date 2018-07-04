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

#include "ThreadPool.h"

//namespace std
//{
//#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
//
//#else
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
//#endif
//}

namespace lava
{
  namespace utility
  {
    void Thread::queueLoop( void )
    {
      while ( true )
      {
        std::function<void( )> task;
        {
          std::unique_lock<std::mutex> lock( queueMutex );
          condition.wait( lock, 
            [ this ] { return stop || !tasks.empty( ); } );
          if ( stop )
          {
            break;
          }
          task = std::move( tasks.front( ) );
        }

        task( );

        {
          std::lock_guard<std::mutex> lock( queueMutex );
          tasks.pop( );
          condition.notify_one( );
        }
      }
    }

    Thread::Thread( void )
    {
      worker = std::thread( &Thread::queueLoop, this );
    }

    Thread::~Thread( void )
    {
      if ( worker.joinable( ) )
      {
        wait( );
        queueMutex.lock( );
        stop = true;
        condition.notify_one( );
        queueMutex.unlock( );
        worker.join( );
      }
    }

    void Thread::addJob( std::function<void( )> function )
    {
      std::lock_guard<std::mutex> lock( queueMutex );
      tasks.push( std::move( function ) );
      condition.notify_one( );
    }

    void Thread::wait( void )
    {
      std::unique_lock<std::mutex> lock( queueMutex );
      condition.wait( lock, [ this ]( ) { return tasks.empty( ); } );
    }

    void ThreadPool::setThreadCount( uint32_t count )
    {
      workers.clear( );
      for ( uint32_t i = 0; i < count; ++i )
      {
        workers.push_back( make_unique< Thread >( ) );
      }
    }
    void ThreadPool::wait( void )
    {
      for ( auto &w : workers )
      {
        w->wait( );
      }
    }
  }
}
