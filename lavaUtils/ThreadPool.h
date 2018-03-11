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

#ifndef __LAVAUTILS_THREADPOOL__
#define __LAVAUTILS_THREADPOOL__

#include <chrono>
#include <thread>
#include <iostream>
#include <queue>
#include <functional>
#include <mutex>

#include <lavaUtils/api.h>

namespace lava
{
  namespace utility
  {
    class Thread
    {
    private:
      bool stop = false;
      std::thread worker;
      // the task queue
      std::queue<std::function<void( )>> tasks;

      // synchronization
      std::mutex queueMutex;
      std::condition_variable condition;

      void queueLoop( void );

    public:
      LAVAUTILS_API
      Thread( void );

      LAVAUTILS_API
      ~Thread( void );

      LAVAUTILS_API
      void addJob( std::function<void( )> function );

      LAVAUTILS_API
      void wait( void );
    };

    class ThreadPool
    {
    public:
      std::vector<std::unique_ptr<Thread>> workers;

      // Sets the number of workers to be allocted in this pool
      LAVAUTILS_API
      void setThreadCount( uint32_t count );

      // Wait until all workers have finished their work items
      LAVAUTILS_API
      void wait( void );
    };
  }
}

#endif /* __LAVAUTILS_THREADPOOL__ */
