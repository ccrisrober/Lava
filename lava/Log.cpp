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

#include "Log.h"

#include "MemoryUtils.h"

namespace lava
{
  std::unique_ptr< Log::OutputHandler > Log::_outputHandler =
    std::unique_ptr< Log::ConsoleOutputHandler >( new Log::ConsoleOutputHandler( ) );
  int Log::_level = Log::LogLevel::LOG_LEVEL_INFO;

  void Log::ConsoleOutputHandler::print( const std::string& src )
  {
    std::cout << src << std::endl;
  }
  void Log::FileOutputHandler::print( const std::string& src )
  {
    _out << src << std::endl;
  }
  void Log::NullOutputHandler::print( const std::string& )
  {
  }
}
