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

#ifndef __LAVA_LOG__
#define __LAVA_LOG__

#include <lava/api.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <chrono>
#include "StringUtils.hpp"

namespace lava
{
  class Log
  {
  private:
    Log( void ) { }
    virtual ~Log( void ) { }
  public:
    enum LogLevel
    {
      LOG_LEVEL_NONE = -1,    // no output
      LOG_LEVEL_ERROR = 2,    // only errors are output
      LOG_LEVEL_WARNING = 3,  // errors and warnings are output
      LOG_LEVEL_INFO = 4,     // errors, warnings, and informative messages
      LOG_LEVEL_DEBUG = 5,    // errors, warnings, informative messages and debug
      LOG_LEVEL_ALL = 99      // all messages are output
    };
    LAVA_API
    static int getLevel( void ) { return _level; }
    LAVA_API
    static void setLevel( int level ) { _level = level; }
  private:
    static int _level;
  public:
    template<typename ... Args>
    static void error( const std::string& msg, Args && ...args )
    {
      print( LogLevel::LOG_LEVEL_ERROR, "E", msg, std::forward< Args >( args )... );
    }
    template<typename ... Args>
    static void warning( const std::string& msg, Args && ...args )
    {
      print( LogLevel::LOG_LEVEL_WARNING, "W", msg, std::forward< Args >( args )... );
    }
    template<typename ... Args>
    static void info( const std::string& msg, Args && ...args )
    {
      print( LogLevel::LOG_LEVEL_INFO, "I", msg, std::forward< Args >( args )... );
    }
    template<typename ... Args>
    static void debug( const std::string& msg, Args && ...args )
    {
      print( LogLevel::LOG_LEVEL_DEBUG, "D", msg, std::forward< Args >( args )... );
    }
    template< typename ... Args >
    static void print( int level, const std::string& levelStr,
      std::string const &TAG, Args &&... args )
    {
      if ( getLevel( ) >= level && _outputHandler != nullptr )
      {
        auto tp = std::chrono::system_clock::now( );
        auto s = std::chrono::duration_cast< std::chrono::microseconds >( 
          tp.time_since_epoch( ) );
        auto t = ( time_t )( s.count( ) );

        auto str = StringUtils::toString("[", t, "] ",
          levelStr, "/", TAG,// " - ",
          std::forward< Args >( args )...);

        _outputHandler->print( str );
      }
    }

    public:
      class OutputHandler
      {
      public:
        LAVA_API
        virtual ~OutputHandler( void ) { }
        virtual void print( const std::string& src ) = 0;
      };
      class ConsoleOutputHandler : public OutputHandler
      {
      public:
        LAVA_API
        ConsoleOutputHandler( void ) { }
        LAVA_API
        virtual ~ConsoleOutputHandler( void ) { }

        LAVA_API
        virtual void print( const std::string& src ) override;
      };
      class FileOutputHandler : public OutputHandler
      {
      public:
        LAVA_API
        FileOutputHandler( const std::string& path ) 
          : _out( path, std::ios::out ) { }
        LAVA_API
        virtual ~FileOutputHandler( void ) { }

        LAVA_API
        virtual void print( const std::string& src ) override;
      private:
        std::ofstream _out;
      };
      class NullOutputHandler : public OutputHandler
      {
      public:
        LAVA_API
        NullOutputHandler( void ) { }
        LAVA_API
        virtual ~NullOutputHandler( void ) { }

        LAVA_API
        virtual void print( const std::string& ) override;
      };
     
      template< class T, typename ... Args >
      static void setOutputHandler( Args &&... args )
      {
        _outputHandler = std::move(
          std::unique_ptr< T >( new T( std::forward< Args >( args )... ) ) );
      }
  //protected:
  //private:
    static std::unique_ptr< OutputHandler > _outputHandler;
  };
}

#endif  /* __LAVA_LOG__ */
